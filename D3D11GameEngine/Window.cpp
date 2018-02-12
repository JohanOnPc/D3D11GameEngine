#include "Window.h"

using Microsoft::WRL::ComPtr;

namespace { Window* window = 0; }

LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return window->WindowProc(hwnd, msg, wParam, lParam);
}

Window::Window(HINSTANCE hInstance, int nCmdShow, std::wstring WndName) : m_hwnd(nullptr), m_Width(1600), m_Height(900), m_Paused(false), m_Resizing(false)
{
	m_hInstance =		hInstance;
	m_nCmdShow =		nCmdShow;
	m_MainWindowTitle = WndName;
	window =			this;

	WinInit();
	D3D11Init();
	m_Timer.Reset();
}

void Window::WinInit()
{
	WNDCLASSEX wc;
	SecureZeroMemory(&wc, sizeof(WNDCLASSEX));

	wc.cbSize =			sizeof(WNDCLASSEX);
	wc.style =			CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc =	MainWindowProc;
	wc.hInstance =		m_hInstance;
	wc.hCursor =		LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground =	(HBRUSH)COLOR_WINDOW;
	wc.lpszClassName =	L"WindowClass";

	RegisterClassEx(&wc);
		
	RECT wr = { 0, 0, 1600, 900 };
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, false);

	m_hwnd = CreateWindowEx(NULL,
		L"WindowClass",
		m_MainWindowTitle.c_str(),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		wr.right - wr.left,
		wr.bottom - wr.top,
		NULL, NULL,
		m_hInstance,
		NULL);
	ShowWindow(m_hwnd, m_nCmdShow);
	UpdateWindow(m_hwnd);
		
}

void Window::D3D11Init()
{
	UINT CreateDeviceFlags = 0;
#if defined(_DEBUG)
	CreateDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL FeatureLevel;
	HRESULT hr;

	if (FAILED(hr = D3D11CreateDevice(
		NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		CreateDeviceFlags,
		NULL, NULL,
		D3D11_SDK_VERSION,
		m_Device.GetAddressOf(),
		&FeatureLevel,
		m_DeviceContext.GetAddressOf())))
	{
		throw std::exception("CreateDevice Failed");
	}

	m_Device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &MsaaQuality);
	assert(MsaaQuality > 0);


	DXGI_SWAP_CHAIN_DESC SwapChainDesc;
	SecureZeroMemory(&SwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

	SwapChainDesc.BufferDesc.Width =					m_Width;
	SwapChainDesc.BufferDesc.Height =					m_Height;
	SwapChainDesc.BufferDesc.RefreshRate.Numerator =	60;
	SwapChainDesc.BufferDesc.RefreshRate.Denominator =	1;
	SwapChainDesc.BufferDesc.Format =					DXGI_FORMAT_R8G8B8A8_UNORM;
	SwapChainDesc.BufferDesc.ScanlineOrdering =			DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	SwapChainDesc.BufferDesc.Scaling =					DXGI_MODE_SCALING_UNSPECIFIED;
	SwapChainDesc.SampleDesc.Count =					4;
	SwapChainDesc.SampleDesc.Quality =					MsaaQuality - 1;
	SwapChainDesc.BufferUsage =							DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapChainDesc.BufferCount =							1;
	SwapChainDesc.OutputWindow =						m_hwnd;
	SwapChainDesc.Windowed =							true;
	SwapChainDesc.SwapEffect =							DXGI_SWAP_EFFECT_DISCARD;
	SwapChainDesc.Flags =								0;

	ComPtr<IDXGIDevice>		Device;
	ComPtr<IDXGIAdapter>	Adapter;
	ComPtr<IDXGIFactory>	Factory;
	m_Device->QueryInterface(__uuidof(IDXGIDevice), (void**)Device.GetAddressOf());
	Device->GetParent(__uuidof(IDXGIAdapter), (void**)Adapter.GetAddressOf());
	Adapter->GetParent(__uuidof(IDXGIFactory), (void**)Factory.GetAddressOf());
	Factory->CreateSwapChain(m_Device.Get(), &SwapChainDesc, &m_SwapChain);
	Device  =	nullptr;
	Adapter =	nullptr;
	Factory =	nullptr;

	ComPtr<ID3D11Texture2D> BackBuffer;
	m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(BackBuffer.GetAddressOf()));
	if (FAILED(hr = m_Device->CreateRenderTargetView(BackBuffer.Get(), 0, m_RenderTargetView.GetAddressOf())))
		throw std::exception("CreateRenderTargetView Failed");
	BackBuffer = nullptr;

	
	D3D11_TEXTURE2D_DESC DepthStencilDesc;
	SecureZeroMemory(&DepthStencilDesc, sizeof(D3D11_TEXTURE2D_DESC));

	DepthStencilDesc.Width =				m_Width;
	DepthStencilDesc.Height =				m_Height;
	DepthStencilDesc.MipLevels =			1;
	DepthStencilDesc.ArraySize =			1;
	DepthStencilDesc.Format =				DXGI_FORMAT_D24_UNORM_S8_UINT;
	DepthStencilDesc.SampleDesc.Count =		4;
	DepthStencilDesc.SampleDesc.Quality =	MsaaQuality - 1;
	DepthStencilDesc.Usage =				D3D11_USAGE_DEFAULT;
	DepthStencilDesc.BindFlags =			D3D11_BIND_DEPTH_STENCIL;
	DepthStencilDesc.CPUAccessFlags =		0;
	DepthStencilDesc.MiscFlags =			0;

	m_Device->CreateTexture2D(&DepthStencilDesc, 0, m_DepthStencilBuffer.GetAddressOf());
	m_Device->CreateDepthStencilView(m_DepthStencilBuffer.Get(), 0, m_DepthStencilView.GetAddressOf());
	m_DeviceContext->OMSetRenderTargets(1, m_RenderTargetView.GetAddressOf(), m_DepthStencilView.Get());

	m_ScreenViewPort.TopLeftX = 0.0f;
	m_ScreenViewPort.TopLeftY = 0.0f;
	m_ScreenViewPort.Width    =	static_cast<float>(m_Width);
	m_ScreenViewPort.Height   =	static_cast<float>(m_Height);
	m_ScreenViewPort.MinDepth = 0.0f;
	m_ScreenViewPort.MaxDepth = 1.0f;
}

void Window::CalculateFps()
{
	static int FrameCount    =	0;
	static float TimeElapsed =	0.0f;
	FrameCount++;

	if ((m_Timer.Time() - TimeElapsed) >= 1.0f)
	{
		float Fps  = (float)FrameCount;
		float Msps = 1000.0f / Fps;

		std::wostringstream out;
		out.precision(6);
		out << m_MainWindowTitle << L" FPS: " << Fps << L" Frame Time: " << Msps << L" (ms)";
		SetWindowText(m_hwnd, out.str().c_str());

		FrameCount = 0;
		TimeElapsed += 1.0f;
	}
}

bool Window::ProcessMessage()
{
	MSG message = { 0 };

	while (message.message != WM_QUIT)
	{
		if (PeekMessage(&message, nullptr, NULL, NULL, PM_REMOVE))
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		else
		{
			m_Timer.Tick();
			if (!m_Paused)
			{
				return true;
			}
			else
				Sleep(100);			
		}
	}
	return false;
}

void Window::Resize()
{
	assert(m_DeviceContext);
	assert(m_Device);
	assert(m_SwapChain);

	m_RenderTargetView.ReleaseAndGetAddressOf();
	m_DepthStencilView.ReleaseAndGetAddressOf();
	m_DepthStencilBuffer.ReleaseAndGetAddressOf();

	ComPtr<ID3D11Texture2D> BackBuffer;

	m_SwapChain->ResizeBuffers(1, m_Width, m_Height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
	m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(BackBuffer.GetAddressOf()));
	m_Device->CreateRenderTargetView(BackBuffer.Get(), 0, m_RenderTargetView.GetAddressOf());

	D3D11_TEXTURE2D_DESC DepthStencilDesc;
	DepthStencilDesc.Width =				m_Width;
	DepthStencilDesc.Height =				m_Height;
	DepthStencilDesc.MipLevels =			1;
	DepthStencilDesc.ArraySize =			1;
	DepthStencilDesc.Format =				DXGI_FORMAT_D24_UNORM_S8_UINT;
	DepthStencilDesc.SampleDesc.Count =		4;
	DepthStencilDesc.SampleDesc.Quality =	MsaaQuality - 1;
	DepthStencilDesc.Usage =				D3D11_USAGE_DEFAULT;
	DepthStencilDesc.BindFlags =			D3D11_BIND_DEPTH_STENCIL;
	DepthStencilDesc.CPUAccessFlags =		0;
	DepthStencilDesc.MiscFlags =			0;

	m_Device->CreateTexture2D(&DepthStencilDesc, 0, m_DepthStencilBuffer.GetAddressOf());
	m_Device->CreateDepthStencilView(m_DepthStencilBuffer.Get(), 0, m_DepthStencilView.GetAddressOf());
	m_DeviceContext->OMSetRenderTargets(1, m_RenderTargetView.GetAddressOf(), m_DepthStencilView.Get());

	m_ScreenViewPort.TopLeftX = 0;
	m_ScreenViewPort.TopLeftY = 0;
	m_ScreenViewPort.Width    =	static_cast<float>(m_Width);
	m_ScreenViewPort.Height   =	static_cast<float>(m_Height);
	m_ScreenViewPort.MinDepth = 0.0f;
	m_ScreenViewPort.MaxDepth = 1.0f;

	m_DeviceContext->RSSetViewports(1, &m_ScreenViewPort);
}

void Window::DrawScene()
{
	assert(m_DeviceContext);
	assert(m_SwapChain);

	const float color[] = { 0.196078f, 0.8f, 0.196078f, 0.0f };

	m_DeviceContext->ClearRenderTargetView(m_RenderTargetView.Get(), reinterpret_cast<const float*>(&color));
	m_DeviceContext->ClearDepthStencilView(m_DepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	m_SwapChain->Present(0, 0);
}

float Window::GetAspectRatio()
{
	return static_cast<float>(m_Width) / m_Height;
}

LRESULT Window::WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		} 

	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE)
		{
			m_Paused = true;
			m_Timer.Stop();
		}
		else
		{
			m_Paused = false;
			m_Timer.Start();
		}
		return 0;

	case WM_ENTERSIZEMOVE:
		m_Paused =		true;
		m_Resizing =	true;
		m_Timer.Stop();
		return			0;

	case WM_EXITSIZEMOVE:
		m_Paused = false;
		m_Resizing = false;
		m_Timer.Start();
		Resize();
		return 0;

	case WM_MENUCHAR:
		return MAKELRESULT(0, MNC_CLOSE);

	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
		return 0;
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}
