#include "AudioCaptureImpl_WASAPI.h"

#include <projectM-4/projectM.h>

#include <Poco/UnicodeConverter.h>

#include <functiondiscoverykeys_devpkey.h>
#include <mmdeviceapi.h>
#include <objbase.h>

// Helper that centralizes WASAPI enumerator acquisition, device list creation,
// and cleanup for the AudioCaptureImpl WASAPI backend.
class WASAPIDeviceCatalog
{
    public:
        explicit WASAPIDeviceCatalog(const AudioCaptureImpl& audioCaptureImpl)
            : _audioCaptureImpl(audioCaptureImpl)
            , _enumerator(audioCaptureImpl.GetDeviceEnumerator())
        {
        }

        ~WASAPIDeviceCatalog()
        {
            // Release the COM enumerator automatically when this object goes out of scope.
            if (_enumerator)
            {
                _enumerator->Release();
            }
        }

        std::vector<AudioCaptureImpl::AudioDevice> DeviceList() const
        {
            if (_enumerator == nullptr)
            {
                return {};
            }

            // Delegate actual device list construction to the single source of truth.
            return _audioCaptureImpl.GetAudioDeviceList(_enumerator);
        }

    private:
        const AudioCaptureImpl& _audioCaptureImpl;
        IMMDeviceEnumerator* _enumerator{nullptr};
};

AudioCaptureImpl::AudioCaptureImpl()
    : _captureThread(this, &AudioCaptureImpl::CaptureThread)
{
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
}

AudioCaptureImpl::~AudioCaptureImpl()
{
    CoUninitialize();
}

std::map<int, std::string> AudioCaptureImpl::AudioDeviceList()
{
    std::map<int, std::string> deviceList{
        {-1, _defaultDeviceName}};

    WASAPIDeviceCatalog deviceCatalog(*this);
    auto captureDevices = deviceCatalog.DeviceList();

    uint32_t index{0};
    for (const auto& device : captureDevices)
    {
        if (device.DeviceId() != nullptr)
        {
            deviceList.insert(std::make_pair(index, device.FriendlyName()));
        }

        index++;
    }

    return deviceList;
}

void AudioCaptureImpl::StartRecording(projectm* projectMHandle, int audioDeviceIndex)
{
    _projectMHandle = projectMHandle;
    _currentAudioDeviceIndex = audioDeviceIndex;

    _isCapturing = true;
    _captureThreadResult = _captureThread();
}

void AudioCaptureImpl::StopRecording()
{
    if (_isCapturing)
    {
        poco_trace(_logger, "Stopping audio capturing thread.");
        _isCapturing = false;
        _fillBufferEvent.set();
        _captureThreadResult.wait();
        poco_trace(_logger, "Audio capturing thread joined.");
    }
}

void AudioCaptureImpl::NextAudioDevice()
{
    StopRecording();

    WASAPIDeviceCatalog deviceCatalog(*this);
    auto captureDevices = deviceCatalog.DeviceList();

    // Will wrap around to loopback capture device (-1).
    int nextAudioDeviceId = ((_currentAudioDeviceIndex + 2) % (static_cast<int>(captureDevices.size()) + 1)) - 1;

    StartRecording(_projectMHandle, nextAudioDeviceId);
}

void AudioCaptureImpl::AudioDeviceIndex(int index)
{
    WASAPIDeviceCatalog deviceCatalog(*this);
    auto captureDevices = deviceCatalog.DeviceList();

    if (index >= -1 && index < static_cast<int>(captureDevices.size()))
    {
        _currentAudioDeviceIndex = index;
        StopRecording();
        StartRecording(_projectMHandle, index);
    }
}

int AudioCaptureImpl::AudioDeviceIndex() const
{
    return _currentAudioDeviceIndex;
}

std::string AudioCaptureImpl::AudioDeviceName() const
{
    if (_currentAudioDeviceIndex < 0)
    {
        return "System Default Audio Device";
    }

    WASAPIDeviceCatalog deviceCatalog(*this);
    auto captureDevices = deviceCatalog.DeviceList();

    if (captureDevices.empty() || _currentAudioDeviceIndex >= static_cast<int>(captureDevices.size()))
    {
        return {};
    }

    return captureDevices.at(_currentAudioDeviceIndex).FriendlyName();
}

void AudioCaptureImpl::FillBuffer()
{
    if (_isCapturing)
    {
        _bufferFilledEvent.reset();
        _fillBufferEvent.set();
        try
        {
            _bufferFilledEvent.wait(20);
        }
        catch (Poco::TimeoutException& ex)
        {
            poco_debug(_logger, "Timeout waiting for audio buffer fill");
        }
    }
}

HRESULT AudioCaptureImpl::QueryInterface(const IID& riid, void** ppvObject)
{
    if (ppvObject == nullptr)
    {
        return E_POINTER;
    }

    if (riid == IID_IUnknown)
    {
        *ppvObject = static_cast<IUnknown*>(this);
        AddRef();
    }
    else if (riid == __uuidof(IMMNotificationClient))
    {
        *ppvObject = static_cast<IMMNotificationClient*>(this);
        AddRef();
    }

    return E_NOINTERFACE;
}

ULONG AudioCaptureImpl::AddRef()
{
    return InterlockedIncrement(&_referenceCount);
}

ULONG AudioCaptureImpl::Release()
{
    return InterlockedDecrement(&_referenceCount);
}

std::string AudioCaptureImpl::UnicodeToString(LPCWSTR unicodeString)
{
    std::string utf8String;
    Poco::UnicodeConverter::convert(std::wstring(unicodeString), utf8String);
    return utf8String;
}

std::vector<AudioCaptureImpl::AudioDevice> AudioCaptureImpl::GetAudioDeviceList(IMMDeviceEnumerator* enumerator) const
{
    auto addEndpoints = [this, enumerator](std::vector<AudioDevice>& deviceList, EDataFlow dataFlow) {
        HRESULT result{S_OK};
        IMMDeviceCollection* audioEndpoints{nullptr};

        result = enumerator->EnumAudioEndpoints(dataFlow, DEVICE_STATE_ACTIVE, &audioEndpoints);
        if (FAILED(result))
        {
            poco_error_f1(_logger, "IMMDeviceEnumerator::EnumAudioEndpoints failed: result = 0x%08?x", result);
            return;
        }

        UINT deviceCount{0};
        audioEndpoints->GetCount(&deviceCount);
        for (UINT item = 0; item < deviceCount; item++)
        {
            IMMDevice* device{nullptr};
            result = audioEndpoints->Item(item, &device);

            if (FAILED(result) || device == nullptr)
            {
                poco_error_f2(_logger, "IMMDeviceEnumerator::Item failed for device %?u: result = 0x%08?x", item, result);
                continue;
            }

            deviceList.emplace_back(device, (dataFlow == eRender));
            device->Release();
        }

        audioEndpoints->Release();
    };

    std::vector<AudioDevice> deviceList;
    addEndpoints(deviceList, eRender);
    addEndpoints(deviceList, eCapture);

    return deviceList;
}

// Helper: Stage 1 - Activate device and get IAudioClient interface
HRESULT AudioCaptureImpl::SelectAndActivateDevice(IMMDevice* device)
{
    HRESULT result = device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, reinterpret_cast<void**>(&_audioClient));
    if (FAILED(result))
    {
        poco_error_f1(_logger, "IMMDevice::Activate(IAudioClient) failed: result = 0x%08?x", result);
    }
    return result;
}

// Helper: Stage 2 - Retrieve and validate audio format
int AudioCaptureImpl::NegotiateAudioFormat()
{
    WAVEFORMATEX* pwfx = nullptr;
    HRESULT result = _audioClient->GetMixFormat(&pwfx);
    if (FAILED(result))
    {
        poco_error_f1(_logger, "IAudioClient::GetMixFormat failed: result = 0x%08?x", result);
        return -1;
    }

    // Validate format is float (IEEE float or WAVE_FORMAT_EXTENSIBLE with float subformat)
    if (pwfx->wFormatTag != WAVE_FORMAT_IEEE_FLOAT)
    {
        auto extensibleFormat = reinterpret_cast<PWAVEFORMATEXTENSIBLE>(pwfx);
        if (pwfx->wFormatTag != WAVE_FORMAT_EXTENSIBLE)
        {
            poco_error_f1(_logger, "IAudioClient::GetMixFormat returned non-float sample format: 0x%04?x", pwfx->wFormatTag);
            CoTaskMemFree(pwfx);
            return -1;
        }
        else if (!IsEqualGUID(KSDATAFORMAT_SUBTYPE_IEEE_FLOAT, extensibleFormat->SubFormat))
        {
            poco_error_f1(_logger, "IAudioClient::GetMixFormat returned non-float extensible sub format: 0x%04?x", extensibleFormat->SubFormat);
            CoTaskMemFree(pwfx);
            return -1;
        }
    }

    int channels = pwfx->nChannels;
    CoTaskMemFree(pwfx);
    return channels;
}

// Helper: Stage 3 - Initialize IAudioClient and get device period
HRESULT AudioCaptureImpl::InitializeAudioClient(int channels, bool useLoopback)
{
    // Get the default device periodicity
    REFERENCE_TIME hnsDefaultDevicePeriod;
    HRESULT result = _audioClient->GetDevicePeriod(&hnsDefaultDevicePeriod, nullptr);
    if (FAILED(result))
    {
        poco_error_f1(_logger, "IAudioClient::GetDevicePeriod failed: result = 0x%08?x", result);
        return result;
    }

    // Get format again for Initialize call
    WAVEFORMATEX* pwfx = nullptr;
    result = _audioClient->GetMixFormat(&pwfx);
    if (FAILED(result))
    {
        poco_error_f1(_logger, "IAudioClient::GetMixFormat failed (second call): result = 0x%08?x", result);
        return result;
    }

    // Initialize the audio client
    // Can't use event-driven processing in loopback mode, but as we
    // get a "fill buffer" request before rendering each frame, this isn't
    // really necessary anyway.
    result = _audioClient->Initialize(
        AUDCLNT_SHAREMODE_SHARED,
        useLoopback ? AUDCLNT_STREAMFLAGS_LOOPBACK : 0,
        hnsDefaultDevicePeriod,
        0,
        pwfx,
        nullptr);

    CoTaskMemFree(pwfx);

    if (FAILED(result))
    {
        poco_error_f1(_logger, "IAudioClient::Initialize failed: result = 0x%08?x", result);
    }
    return result;
}

// Helper: Stage 4 - Setup capture client and start stream
HRESULT AudioCaptureImpl::SetupCaptureAndStream()
{
    // Get the capture client interface
    HRESULT result = _audioClient->GetService(
        __uuidof(IAudioCaptureClient),
        (void**)&_audioCaptureClient);

    if (FAILED(result))
    {
        poco_error_f1(_logger, "IAudioClient::GetService failed: result = 0x%08?x", result);
        return result;
    }

    // Start the audio stream
    result = _audioClient->Start();
    if (FAILED(result))
    {
        poco_error_f1(_logger, "IAudioClient::Start failed: result = 0x%08?x", result);
    }
    return result;
}

bool AudioCaptureImpl::OpenAudioDevice(IMMDevice* device, bool useLoopback)
{
    // Stage 1: Activate device and get IAudioClient
    HRESULT result = SelectAndActivateDevice(device);
    if (FAILED(result))
    {
        goto cleanup;
    }

    // Stage 2: Negotiate audio format
    int channels = NegotiateAudioFormat();
    if (channels <= 0)
    {
        goto cleanup;
    }
    _channels = channels;

    // Stage 3: Initialize audio client
    result = InitializeAudioClient(channels, useLoopback);
    if (FAILED(result))
    {
        goto cleanup;
    }

    // Stage 4: Setup capture client and start stream
    result = SetupCaptureAndStream();
    if (FAILED(result))
    {
        goto cleanup;
    }

    return true;

cleanup:
    // Consistent cleanup on any failure: stops stream and releases resources
    if (_audioClient)
    {
        poco_trace(_logger, "Cleaning up after failed device initialization.");
        _audioClient->Stop();
    }
    if (_audioCaptureClient)
    {
        poco_trace(_logger, "Releasing audio capture client (cleanup).");
        _audioCaptureClient->Release();
        _audioCaptureClient = nullptr;
    }
    if (_audioClient)
    {
        poco_trace(_logger, "Releasing audio client (cleanup).");
        _audioClient->Release();
        _audioClient = nullptr;
    }
    return false;
}

void AudioCaptureImpl::CloseAudioDevice(IMMDevice* device)
{
    if (_audioClient)
    {
        poco_trace(_logger, "Stopping audio client.");
        _audioClient->Stop();
    }

    if (_audioCaptureClient)
    {
        poco_trace(_logger, "Releasing audio capture client.");
        _audioCaptureClient->Release();
        _audioCaptureClient = nullptr;
    }
    if (_audioClient)
    {
        poco_trace(_logger, "Releasing audio client.");
        _audioClient->Release();
        poco_trace(_logger, "Audio client released.");
        _audioClient = nullptr;
    }

    if (device)
    {
        poco_trace(_logger, "Releasing audio device.");
        device->Release();
    }
}

void AudioCaptureImpl::CaptureThread()
{
    poco_debug(_logger, "Audio capture thread starting.");

    IMMDeviceEnumerator* enumerator{InitializeCaptureThread()};
    if (enumerator == nullptr)
    {
        return;
    }

    do
    {
        _restartCapturing = false;

        IMMDevice* device{nullptr};
        std::string deviceName;
        bool useLoopback{false};

        if (!SelectAndOpenDevice(enumerator, &device, deviceName, useLoopback))
        {
            continue;
        }

        PerformAudioCapture();
        CloseDeviceAndCleanup(device);

        poco_debug(_logger, "Audio device closed.");
    } while (_restartCapturing);

    CleanupCaptureThread(enumerator);

    poco_debug(_logger, "Audio capture thread exiting.");
}

IMMDeviceEnumerator* AudioCaptureImpl::InitializeCaptureThread()
{
    HRESULT result = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    if (FAILED(result))
    {
        poco_error_f1(_logger, "CoInitializeEx() failed: result = 0x%08?x", result);
        throw std::bad_alloc();
    }

    IMMDeviceEnumerator* enumerator{GetDeviceEnumerator()};

    if (enumerator == nullptr)
    {
        CoUninitialize();
        throw std::bad_alloc();
    }

    poco_trace(_logger, "Registering device callbacks.");
    enumerator->RegisterEndpointNotificationCallback(this);

    return enumerator;
}

bool AudioCaptureImpl::SelectAndOpenDevice(IMMDeviceEnumerator* enumerator, IMMDevice** device, std::string& deviceName, bool& useLoopback)
{
    auto devices{GetAudioDeviceList(enumerator)};
    useLoopback = true;

    if (_currentAudioDeviceIndex == -1 || _currentAudioDeviceIndex >= devices.size())
    {
        // Get the default render endpoint for opening it as a loopback device.
        HRESULT result = enumerator->GetDefaultAudioEndpoint(eRender, eConsole, device);
        if (FAILED(result))
        {
            poco_error_f1(_logger, "IMMDeviceEnumerator::GetDefaultAudioEndpoint failed: result = 0x%08?x", result);
            if (*device)
            {
                (*device)->Release();
                *device = nullptr;
            }
            return false;
        }

        deviceName = _defaultDeviceName;
    }
    else
    {
        // Get a device by its ID according to the currently selected index.
        useLoopback = devices.at(_currentAudioDeviceIndex).IsRenderDevice();
        deviceName = devices.at(_currentAudioDeviceIndex).FriendlyName();
        HRESULT result = enumerator->GetDevice(devices.at(_currentAudioDeviceIndex).DeviceId(), device);

        if (FAILED(result))
        {
            poco_error_f1(_logger, "IMMDeviceEnumerator::GetDevice failed: result = 0x%08?x", result);
            if (*device)
            {
                (*device)->Release();
                *device = nullptr;
            }
            return false;
        }
    }

    LPWSTR deviceID{nullptr};
    HRESULT result = (*device)->GetId(&deviceID);
    if (FAILED(result))
    {
        poco_error_f1(_logger, "IMMDevice::GetId failed: result = 0x%08?x", result);
    }
    else
    {
        _currentCaptureDeviceId = UnicodeToString(deviceID);
    }

    if (!OpenAudioDevice(*device, useLoopback))
    {
        _isCapturing = false;
        if (*device)
        {
            CloseAudioDevice(*device);
            *device = nullptr;
        }
        return false;
    }

    poco_information_f3(_logger, "Audio device opened: %s (channels: %hu, loopback: %b)", deviceName, _channels, useLoopback);
    return true;
}

void AudioCaptureImpl::PerformAudioCapture()
{
    while (_isCapturing && !_restartCapturing)
    {
        try
        {
            _fillBufferEvent.tryWait(500);
        }
        catch (Poco::TimeoutException& ex)
        {
            poco_debug(_logger, "FillBuffer event timeout, proceeding to flush buffer or abort.");
        }

        if (!_isCapturing)
        {
            break;
        }

        UINT32 packetLength;

        _audioCaptureClient->GetNextPacketSize(&packetLength);
        while (packetLength != 0)
        {
            BYTE* data;
            UINT32 framesAvailable;
            DWORD flags;

            HRESULT result = _audioCaptureClient->GetBuffer(&data, &framesAvailable, &flags, nullptr, nullptr);
            if (FAILED(result))
            {
                poco_error_f1(_logger, "IAudioCaptureClient::GetBuffer failed: result = 0x%08?x", result);
                _isCapturing = false;
                break;
            }

            if (flags & AUDCLNT_BUFFERFLAGS_SILENT)
            {
                data = nullptr;
            }

            poco_trace_f1(_logger, "Audio frames available for capturing: %u", data ? framesAvailable : 0);

            if (framesAvailable > 0 && data != nullptr)
            {
                projectm_pcm_add_float(_projectMHandle, reinterpret_cast<float*>(data), framesAvailable, static_cast<projectm_channels>(_channels));
            }

            _audioCaptureClient->ReleaseBuffer(framesAvailable);

            _audioCaptureClient->GetNextPacketSize(&packetLength);
        }

        _bufferFilledEvent.set();
    }
}

void AudioCaptureImpl::CloseDeviceAndCleanup(IMMDevice* device)
{
    CloseAudioDevice(device);
}

void AudioCaptureImpl::CleanupCaptureThread(IMMDeviceEnumerator* enumerator)
{
    poco_trace(_logger, "Unregistering device callbacks.");
    enumerator->UnregisterEndpointNotificationCallback(this);
    poco_trace(_logger, "Releasing audio device enumerator.");
    enumerator->Release();
    poco_trace(_logger, "Audio device enumerator released.");

    CoUninitialize();
}

IMMDeviceEnumerator* AudioCaptureImpl::GetDeviceEnumerator() const
{
    IMMDeviceEnumerator* enumerator{nullptr};

    HRESULT result = CoCreateInstance(
        __uuidof(MMDeviceEnumerator),
        nullptr,
        CLSCTX_ALL,
        __uuidof(IMMDeviceEnumerator),
        (void**) &enumerator);

    if (FAILED(result))
    {
        poco_error_f1(_logger, "CoCreateInstance(IMMDeviceEnumerator) failed: result = 0x%08?x", result);
    }

    return enumerator;
}

HRESULT AudioCaptureImpl::OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState)
{
    auto deviceId{UnicodeToString(pwstrDeviceId)};

    poco_trace_f2(_logger, "Audio device state changed for device ID %s: %lu", deviceId, dwNewState);

    // Recalculate current device index and restart only if not default.
    if (_currentAudioDeviceIndex >= 0)
    {
        WASAPIDeviceCatalog deviceCatalog(*this);
        auto captureDevices = deviceCatalog.DeviceList();

        for (int index = 0; index < captureDevices.size(); ++index)
        {
            if (UnicodeToString(captureDevices.at(index).DeviceId()) == deviceId)
            {
                _currentAudioDeviceIndex = index;
                break;
            }
        }

        // Restart capturing only in case the current device state changed.
        if (_isCapturing && deviceId == _currentCaptureDeviceId)
        {
            _restartCapturing = true;
        }
    }

    return S_OK;
}

HRESULT AudioCaptureImpl::OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDefaultDeviceId)
{
    poco_trace_f1(_logger, "Default audio device changed to ID %s", UnicodeToString(pwstrDefaultDeviceId));

    if (flow != eRender || _currentAudioDeviceIndex != -1)
    {
        return S_OK;
    }

    if (_isCapturing)
    {
        _restartCapturing = true;
    }

    return S_OK;
}

HRESULT AudioCaptureImpl::OnDeviceAdded(LPCWSTR pwstrDeviceId)
{
    poco_trace_f1(_logger, "Audio device added: %s", UnicodeToString(pwstrDeviceId));

    return S_OK;
}

HRESULT AudioCaptureImpl::OnDeviceRemoved(LPCWSTR pwstrDeviceId)
{
    poco_trace_f1(_logger, "Audio device removed: %s", UnicodeToString(pwstrDeviceId));

    return S_OK;
}

HRESULT AudioCaptureImpl::OnPropertyValueChanged(LPCWSTR pwstrDeviceId, const PROPERTYKEY key)
{
    poco_trace_f1(_logger, "Audio device property changed for device ID %s", UnicodeToString(pwstrDeviceId));

    return S_OK;
}

AudioCaptureImpl::AudioDevice::AudioDevice(IMMDevice* device, bool isRenderDevice)
    : _friendlyName(GetAudioEndpointFriendlyName(device))
    , _isRenderDevice(isRenderDevice)
{
    if (device != nullptr)
    {
        HRESULT result = device->GetId(&_deviceId);

        if (FAILED(result))
        {
            poco_trace_f1(_logger, "IMMDevice::GetId failed: result = 0x%08?x", result);
        }

        poco_trace_f3(_logger, R"(Added WASAPI audio device "%s" with ID %s (Render device: %b))",
                      _friendlyName, AudioCaptureImpl::UnicodeToString(_deviceId), _isRenderDevice);
    }
}

AudioCaptureImpl::AudioDevice::AudioDevice(AudioCaptureImpl::AudioDevice&& other) noexcept
{
    _deviceId = other._deviceId;
    other._deviceId = nullptr;
    _friendlyName = std::move(other._friendlyName);
    _isRenderDevice = other._isRenderDevice;
}

AudioCaptureImpl::AudioDevice::~AudioDevice()
{
    if (_deviceId)
    {
        CoTaskMemFree(_deviceId);
        _deviceId = nullptr;
    }
}

std::string AudioCaptureImpl::AudioDevice::GetAudioEndpointFriendlyName(IMMDevice* pMMDevice)
{
    HRESULT result{S_OK};

    if (pMMDevice == nullptr)
    {
        return AudioCaptureImpl::_defaultDeviceName;
    }

    IPropertyStore* deviceProps{nullptr};
    result = pMMDevice->OpenPropertyStore(STGM_READ, &deviceProps);
    if (FAILED(result) || deviceProps == nullptr)
    {
        poco_error_f1(_logger, "IMMDevice::OpenPropertyStore failed: result = 0x%08?x", result);
        return {};
    }

    PROPVARIANT variantName;
    PropVariantInit(&variantName);

    result = deviceProps->GetValue(PKEY_Device_FriendlyName, &variantName);
    if (FAILED(result))
    {
        deviceProps->Release();
        poco_error_f1(_logger, "IMMDevice::OpenPropertyStore failed: result = 0x%08?x", result);
        return {};
    }

    std::string deviceFriendlyName = AudioCaptureImpl::UnicodeToString(variantName.pwszVal);

    PropVariantClear(&variantName);
    deviceProps->Release();

    return deviceFriendlyName;
}

LPWSTR AudioCaptureImpl::AudioDevice::DeviceId() const
{
    return _deviceId;
}

std::string AudioCaptureImpl::AudioDevice::FriendlyName() const
{
    return _friendlyName;
}

bool AudioCaptureImpl::AudioDevice::IsRenderDevice() const
{
    return _isRenderDevice;
}
