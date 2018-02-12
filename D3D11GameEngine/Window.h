#pragma once

#include <Windows.h>
#include <windowsx.h>
#include <wrl.h>
#include <d3d11.h>
#include <d3d10.h>
#include <dxgi.h>
#include <cassert>
#include <exception>
#include <string>
#include <sstream>

#include "Timer.h"

#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3d10.lib")

class Window
{
public:
	Window(HINSTANCE, int, std::wstring);
	bool	ProcessMessage();
	void	CalculateFps();
	void	DrawScene();
	LRESULT WindowProc(HWND, UINT, WPARAM, LPARAM);

private:
	void	WinInit();
	void	D3D11Init();
	void	Resize();
	float	GetAspectRatio();

private:
	//Win32 variables
	HWND			m_hwnd;
	HINSTANCE		m_hInstance;
	int				m_nCmdShow;
	int				m_Width, m_Height;
	std::wstring	m_MainWindowTitle;

	//D3D11 Pointers
	Microsoft::WRL::ComPtr<IDXGISwapChain>			m_SwapChain;
	Microsoft::WRL::ComPtr<ID3D11Device>			m_Device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext>		m_DeviceContext;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView>	m_RenderTargetView;
	Microsoft::WRL::ComPtr<ID3D11Texture2D>			m_DepthStencilBuffer;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView>	m_DepthStencilView;
	D3D11_VIEWPORT									m_ScreenViewPort;

	//Other variables
	Timer	m_Timer;
	bool	m_Paused;
	bool	m_Resizing;
	UINT	MsaaQuality;
};


