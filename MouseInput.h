#pragma once

#include <atlbase.h>
#include <initguid.h>
#include <dinput.h>

class MouseInput
{
public:
	typedef void(__stdcall* MOUSE_CALLBACK)(int, int, int, int, double, void*);
private:
	BOOL m_running;
	HINSTANCE m_hinst;
	MOUSE_CALLBACK m_callback;
	LPVOID m_param;
	HANDLE m_event, m_thread;
protected:
	static unsigned long __stdcall run(MouseInput* object)
	{
		if (object != NULL)
		{
			object->run();
		}

		return 0;
	}

	virtual void run()
	{
		ATL::CComPtr<IDirectInput8> input;
		ATL::CComPtr<IDirectInputDevice8> device;
		DIMOUSESTATE state;
		HANDLE events[2];
		HRESULT hr;
		LARGE_INTEGER freq, prev, next;
		double delay;
		int dx, dy, dz, btns;

		events[0] = m_event;
		events[1] = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (events[1] == NULL) goto OnExit;

		hr = DirectInput8Create(m_hinst, DIRECTINPUT_VERSION,
			IID_IDirectInput8, (void**)&input, NULL);
		if (FAILED(hr)) goto OnExit;

		hr = input->CreateDevice(GUID_SysMouse, &device, NULL);
		if (FAILED(hr)) goto OnExit;

		hr = device->SetDataFormat(&c_dfDIMouse);
		if (FAILED(hr)) goto OnExit;

		hr = device->SetCooperativeLevel(GetDesktopWindow(),
			DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);
		if (FAILED(hr)) goto OnExit;

		hr = device->SetEventNotification(events[1]);
		if (FAILED(hr)) goto OnExit;

		hr = device->Acquire();
		if (FAILED(hr)) goto OnExit;

		QueryPerformanceFrequency(&freq);
		QueryPerformanceCounter(&prev);

		while (m_running)
		{
			if (WaitForMultipleObjects(_countof(events), events,
				FALSE, INFINITE) != (WAIT_OBJECT_0 + 1))
			{
				break;
			}

			hr = device->GetDeviceState(sizeof(state), &state);
			if (FAILED(hr)) continue;

			QueryPerformanceCounter(&next);
			delay = (double)(next.QuadPart - prev.QuadPart) / freq.QuadPart;
			prev = next;

			dx = state.lX;
			dy = state.lY;
			dz = state.lZ;
			btns = 0;
			if (state.rgbButtons[0] & 0x80) btns |= MK_LBUTTON;
			if (state.rgbButtons[1] & 0x80) btns |= MK_RBUTTON;
			if (state.rgbButtons[2] & 0x80) btns |= MK_MBUTTON;

			if (m_callback != NULL)
			{
				m_callback(dx, dy, dz, btns, delay, m_param);
			}
		}

		hr = device->Unacquire();
		if (FAILED(hr)) goto OnExit;

		hr = S_OK;
	OnExit:
		device.Release();
		input.Release();

		if (events[1] != NULL)
		{
			CloseHandle(events[1]);
		}
	}
public:
	MouseInput()
	{
		m_running = FALSE;
		m_hinst = NULL;
		m_callback = NULL;
		m_param = NULL;
		m_event = NULL;
		m_thread = NULL;
	}

	virtual ~MouseInput()
	{
		close();
	}

	virtual bool open(HINSTANCE hinst, MOUSE_CALLBACK callback, LPVOID param)
	{
		if (hinst == NULL)
		{
			hinst = GetModuleHandle(NULL);
		}

		if (m_thread == NULL)
		{
			m_running = TRUE;
			m_hinst = hinst;
			m_callback = callback;
			m_param = param;
			m_event = CreateEvent(NULL, FALSE, FALSE, NULL);
			m_thread = CreateThreadT(NULL, 0, run, this, 0, NULL);
		}

		return m_thread != NULL;
	}

	virtual void close()
	{
		m_running = FALSE;

		if (m_event != NULL)
		{
			SetEvent(m_event);
		}

		if (m_thread != NULL)
		{
			WaitForSingleObject(m_thread, INFINITE);
			CloseHandle(m_thread);
		}

		if (m_event != NULL)
		{
			CloseHandle(m_event);
		}

		m_hinst = NULL;
		m_callback = NULL;
		m_event = NULL;
		m_thread = NULL;
	}

	virtual bool is_opened() const
	{
		return m_thread != NULL;
	}
};
