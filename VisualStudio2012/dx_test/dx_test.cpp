// -----------------------------------------------------------------------
// five-pixel-font library
// by Chris Gassib
// -----------------------------------------------------------------------
// This is free and unencumbered software released into the public domain.
// 
// Anyone is free to copy, modify, publish, use, compile, sell, or
// distribute this software, either in source code form or as a compiled
// binary, for any purpose, commercial or non-commercial, and by any
// means.
// 
// In jurisdictions that recognize copyright laws, the author or authors
// of this software dedicate any and all copyright interest in the
// software to the public domain. We make this dedication for the benefit
// of the public at large and to the detriment of our heirs and
// successors. We intend this dedication to be an overt act of
// relinquishment in perpetuity of all present and future rights to this
// software under copyright law.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
// 
// For more information, please refer to <http://unlicense.org/>
//

#include "stdafx.h"

#define FPF_IMPLEMENTATION
#include "five_pixel_font.h"

#define MAX_LOADSTRING 100
#define CUSTOMFVF (D3DFVF_XYZ | D3DFVF_TEX1 )

struct CUSTOMVERTEX
{
    FLOAT x, y, z;  // from the D3DFVF_XYZ flag
    FLOAT u, v;     // D3DFVF_TEX1
};

const int ClientWidth = 640;
const int ClientHeight = 640;

// Global Variables:
HINSTANCE hInst;                        // current instance
HWND hWnd;
const TCHAR szTitle[]       = _T("Five Pixel Font Test"); // The title bar text
const TCHAR szWindowClass[] = _T("FpfTestWindow");        // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

// global declarations
LPDIRECT3D9 d3d;            // the pointer to our Direct3D interface
LPDIRECT3DDEVICE9 d3ddev;   // the pointer to the device class
LPDIRECT3DTEXTURE9 pTexture;
LPDIRECT3DVERTEXBUFFER9 v_buffer;

// function prototypes
void initD3D(HWND hWnd);    // sets up and initializes Direct3D
void render_frame(void);    // renders a single frame
void cleanD3D(void);        // closes Direct3D and releases memory

int APIENTRY _tWinMain(
    _In_ HINSTANCE      hInstance,
    _In_opt_ HINSTANCE  hPrevInstance,
    _In_ LPTSTR         lpCmdLine,
    _In_ int            nCmdShow
    )
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Initialize global strings
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    initD3D(hWnd);

    // Main message loop:
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    cleanD3D();

    return (int) msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = nullptr;
    wcex.hCursor        = nullptr;
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = nullptr;
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = nullptr;

    return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // Store instance handle in our global variable

    hWnd = CreateWindow(
        szWindowClass,
        szTitle,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        ClientWidth,
        ClientHeight,
        nullptr,
        nullptr,
        hInstance,
        nullptr
        );

    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    RECT client;
    GetClientRect(hWnd, &client);

    if (client.right != ClientWidth || client.bottom != ClientHeight)
    {
        RECT window;
        GetWindowRect(hWnd, &window);

        const auto cx = (window.right - window.left) - (client.right - ClientWidth);
        const auto cy = (window.bottom - window.top) - (client.bottom - ClientHeight);

        SetWindowPos(hWnd, nullptr, 0, 0, cx, cy, 0);
    }

    return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    //PAINTSTRUCT ps;
    //HDC hdc;

    switch (message)
    {
    case WM_PAINT:
        //hdc = BeginPaint(hWnd, &ps);
        if (nullptr != d3d)
        {
            render_frame();
        }
        //EndPaint(hWnd, &ps);
        break;
    case WM_KEYDOWN:
        if (VK_ESCAPE == wParam)
        {
            DestroyWindow(hWnd);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void init_graphics(void)
{
    float left   = 0.0f;
    float top    = 0.0f;
    float right  = 1.0f;
    float bottom = 1.0f;
    //fpf_get_glyph_dx_texture_coordinates('w', &left, &top, &right, &bottom);
    //fpf_get_cursor_glyph_dx_texture_coordinates(FPF_VERTICAL_CURSOR, &left, &top, &right, &bottom);

    // create three vertices using the CUSTOMVERTEX struct built earlier
    CUSTOMVERTEX vertices[] =
    {
        { -1.0f, -1.0f, 0.0f, left,  bottom, },
        { -1.0f,  1.0f, 0.0f, left,  top,    },
        {  1.0f, -1.0f, 0.0f, right, bottom, },
        {  1.0f,  1.0f, 0.0f, right, top,    },
    };

    // Compensate for pre-DX10 texture coordinate retardation.
    for (auto i = 0; i < (sizeof(vertices) / sizeof(CUSTOMVERTEX)); ++i)
    {
        vertices[i].x -= (1.0f / float(ClientWidth)) * 0.5f;
        vertices[i].y -= (1.0f / float(ClientHeight)) * 0.5f;
    }

    // create the vertex and store the pointer into v_buffer, which is created globally
    d3ddev->CreateVertexBuffer(
        4 * sizeof(CUSTOMVERTEX),
        0,
        CUSTOMFVF,
        D3DPOOL_MANAGED,
        &v_buffer,
        nullptr
        );

    // lock and copy the vertices to the buffer
    void* pVoid;
    v_buffer->Lock(0, 0, &pVoid, 0);
    {
        memcpy(pVoid, vertices, sizeof(vertices));
    }
    v_buffer->Unlock();
}

// this function initializes and prepares Direct3D for use
void initD3D(HWND hWnd)
{
    d3d = Direct3DCreate9(D3D_SDK_VERSION);    // create the Direct3D interface

    D3DPRESENT_PARAMETERS d3dpp;    // create a struct to hold various device information

    ZeroMemory(&d3dpp, sizeof(d3dpp));    // clear out the struct for use
    d3dpp.Windowed = TRUE;    // program windowed, not fullscreen
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;    // discard old frames
    d3dpp.hDeviceWindow = hWnd;    // set the window to be used by Direct3D

    // create a device class using this information and information from the d3dpp stuct
    d3d->CreateDevice(
        D3DADAPTER_DEFAULT,
        D3DDEVTYPE_HAL,
        hWnd,
        D3DCREATE_SOFTWARE_VERTEXPROCESSING,
        &d3dpp,
        &d3ddev
        );

    // Turn off DX lighting (which is ON by default).
    d3ddev->SetRenderState(D3DRS_LIGHTING, FALSE);

    // Turn on alpha blending.
    d3ddev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    d3ddev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    d3ddev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);

    // Set the magic DX "Texture Factor" color to the desired font color and opacity.
    d3ddev->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_ARGB(255, 0, 255, 0));

    // Draw with the Texture Factor color.
    d3ddev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    d3ddev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);

    // Modulate the alpha texture with the Texture Factor's alpha component.
    d3ddev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    d3ddev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    d3ddev->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);

    d3ddev->CreateTexture(
        FPF_TEXTURE_WIDTH,
        FPF_TEXTURE_HEIGHT,
        1,
        0,
        D3DFMT_A8,
        D3DPOOL_MANAGED,
        &pTexture,
        nullptr
        );

    D3DLOCKED_RECT rect;
    pTexture->LockRect(0, &rect, nullptr, 0);
    {
        auto result = fpf_create_alpha_texture(
            static_cast<unsigned char*>(rect.pBits),
            FPF_TEXTURE_WIDTH * FPF_TEXTURE_HEIGHT,
            rect.Pitch,
            FPF_RASTER_Y_AXIS
            );
        if (0 != result)
        {
            DebugBreak();
        }
    }
    pTexture->UnlockRect(0);

    d3ddev->SetTexture(0, pTexture);
}

// this is the function used to render a single frame
void render_frame(void)
{
    init_graphics();

    // clear the window to a deep blue
    d3ddev->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 40, 100), 1.0f, 0);

    d3ddev->BeginScene();    // begins the 3D scene
    {

        // select which vertex format we are using
        d3ddev->SetFVF(CUSTOMFVF);

        // select the vertex buffer to display
        d3ddev->SetStreamSource(0, v_buffer, 0, sizeof(CUSTOMVERTEX));

        // copy the vertex buffer to the back buffer
        d3ddev->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);

    }
    d3ddev->EndScene();    // ends the 3D scene

    d3ddev->Present(NULL, NULL, NULL, NULL);    // displays the created frame
}

// this is the function that cleans up Direct3D and COM
void cleanD3D(void)
{
    v_buffer->Release();    // close and release the vertex buffer
    pTexture->Release();
    d3ddev->Release();    // close and release the 3D device
    d3d->Release();    // close and release Direct3D
}
