// 기본 Windows 헤더 파일과 Direct3D 헤더 파일을 포함합니다. 
#include <windows.h> 
#include <windowsx.h> 
#include <d3d11.h> 
#include <d3dx11.h> 
#include <d3dx10.h> 

// Direct3D 라이브러리 파일을 포함합니다. 
#pragma comment (lib, "d3d11.lib") 
#pragma comment (lib, "d3dx11.lib") 
#pragma comment (lib, "d3dx10.lib") 

// 화면 해상도를 정의합니다. 
#define SCREEN_WIDTH 800 
#define SCREEN_HEIGHT 600 

// 전역 선언 
IDXGISwapChain* swapchain; // 스왑 체인 인터페이스에 대한 포인터 
ID3D11Device* dev; // Direct3D 장치 인터페이스에 대한 포인터 
ID3D11DeviceContext* devcon; // Direct3D 장치 컨텍스트에 대한 포인터 
ID3D11RenderTargetView* backbuffer; // 백 버퍼에 대한 포인터 
ID3D11DepthStencilView* zbuffer; // 깊이 버퍼에 대한 포인터 
ID3D11InputLayout* pLayout; // 입력 레이아웃에 대한 포인터 
ID3D11VertexShader* pVS; // 정점 셰이더에 대한 포인터 
ID3D11PixelShader* pPS; // 픽셀 셰이더에 대한 포인터 
ID3D11Buffer* pVBuffer; // 정점 버퍼에 대한 포인터 
ID3D11Buffer* pCBuffer; // 상수 버퍼에 대한 포인터 
ID3D11Buffer* pIBuffer; // 인덱스 버퍼에 대한 포인터

// 다양한 버퍼 구조체 
struct VERTEX { FLOAT X, Y, Z; D3DXCOLOR Color; };
struct PERFRAME { D3DXCOLOR Color; FLOAT X, Y, Z; };

// 함수 프로토타입 
void InitD3D(HWND hWnd); // Direct3D를 설정하고 초기화합니다 
void RenderFrame(void); // 단일 프레임을 렌더링합니다 
void CleanD3D(void); // Direct3D를 닫고 메모리를 해제합니다 
void InitGraphics(void); // 렌더링할 모양을 만듭니다 
void InitPipeline(void); // 셰이더를 로드하고 준비합니다 

// WindowProc 함수 프로토타입 
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// 모든 Windows 프로그램의 진입점 
int WINAPI WinMain(HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow)
{
    HWND hWnd;
    WNDCLASSEX wc;

    ZeroMemory(&wc, sizeof(WNDCLASSEX));

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = L"WindowClass";

    RegisterClassEx(&wc);

    RECT wr = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

    hWnd = CreateWindowEx(NULL,
        L"WindowClass",
        L"첫 번째 Direct3D 프로그램",
        WS_OVERLAPPEDWINDOW,
        300,
        300,
        wr.right - wr.left,
        wr.bottom - wr.top,
        NULL,
        NULL,
        hInstance,
        NULL);

    ShowWindow(hWnd, nCmdShow);

    // Direct3D 설정 및 초기화 
    InitD3D(hWnd);

    // 메인 루프로 이동: 

    MSG msg;

    while (TRUE)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if (msg.message == WM_QUIT)
                break;
        }

        RenderFrame();
    }

    // DirectX와 COM 정리 
    CleanD3D();

    return msg.wParam;
}


// 이것은 프로그램의 주요 메시지 핸들러입니다. 
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_DESTROY:
    {
        PostQuitMessage(0);
        return 0;
    } break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}


// 이 함수는 Direct3D를 초기화하고 사용할 수 있도록 준비합니다. 
void InitD3D(HWND hWnd)
{
    // 스왑 체인에 대한 정보를 보관할 구조체를 만듭니다. 
    DXGI_SWAP_CHAIN_DESC scd;

    // 사용할 수 있도록 구조체를 지웁니다. 
    ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

    // 스왑 체인 설명 채우기 struct 
    scd.BufferCount = 1; // 백 버퍼 하나 
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // 32비트 색상 사용 
    scd.BufferDesc.Width = SCREEN_WIDTH; // 백 버퍼 너비 설정 
    scd.BufferDesc.Height = SCREEN_HEIGHT; // 백 버퍼 높이 설정 
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // 스왑 체인 사용 방법 
    scd.OutputWindow = hWnd; // 사용할 창
    scd.SampleDesc.Count = 4; // 멀티샘플 수 
    scd.Windowed = TRUE; // 창 모드/전체 화면 모드 
    scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; // 전체 화면 전환 허용 

    // scd 구조체에 있는 정보를 사용하여 장치, 장치 컨텍스트 및 스왑 체인 생성 
    D3D11CreateDeviceAndSwapChain(NULL,
        D3D_DRIVER_TYPE_HARDWARE,
        NULL,
        NULL,
        NULL,
        NULL,
        D3D11_SDK_VERSION,
        &scd,
        &swapchain,
        &dev,
        NULL,
        &devcon);


    // 깊이 버퍼 텍스처 생성 
    D3D11_TEXTURE2D_DESC texd;
    ZeroMemory(&texd, sizeof(texd));

    texd.Width = SCREEN_WIDTH;
    texd.Height = SCREEN_HEIGHT;
    texd.ArraySize = 1;
    texd.MipLevels = 1;
    texd.SampleDesc.Count = 4;
    texd.Format = DXGI_FORMAT_D32_FLOAT;
    texd.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    ID3D11Texture2D* pDepthBuffer;
    dev->CreateTexture2D(&texd, NULL, &pDepthBuffer);

    // 깊이 버퍼 생성 
    D3D11_DEPTH_STENCIL_VIEW_DESC dsvd;
    ZeroMemory(&dsvd, sizeof(dsvd));

    dsvd.Format = DXGI_FORMAT_D32_FLOAT;
    dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;

    dev->CreateDepthStencilView(pDepthBuffer, &dsvd, &zbuffer);
    pDepthBuffer->Release();

    // 백 버퍼의 주소를 가져옵니다 
    ID3D11Texture2D* pBackBuffer;
    swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

    // 백 버퍼 주소를 사용하여 렌더 타겟을 생성합니다 
    dev->CreateRenderTargetView(pBackBuffer, NULL, &backbuffer);
    pBackBuffer->Release();

    // 렌더 타겟을 백 버퍼로 설정합니다 
    devcon->OMSetRenderTargets(1, &backbuffer, zbuffer);


    // 뷰포트를 설정합니다 
    D3D11_VIEWPORT viewport;
    ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = SCREEN_WIDTH;
    viewport.Height = SCREEN_HEIGHT;
    viewport.MinDepth = 0; // 객체가 깊이 버퍼에 가장 가까이 있을 수 있는 최대 거리는 0.0입니다.
    viewport.MaxDepth = 1; // 객체가 깊이 버퍼에서 가장 멀리 있을 수 있는 최대 거리는 1.0입니다. 

    devcon->RSSetViewports(1, &viewport);

    InitPipeline();
    InitGraphics();
}


// 이것은 단일 프레임을 렌더링하는 데 사용되는 함수입니다. 
void RenderFrame(void)
{
    D3DXMATRIX matRotate, matView, matProjection;
    D3DXMATRIX matFinal;

    static float Time = 0.0f; Time += 0.0001f;

    // 월드 행렬을 생성합니다. 
    D3DXMatrixRotationY(&matRotate, Time);

    // 뷰 행렬 생성 
    D3DXVECTOR3 position(0.0f, 9.0f, 24.0f);
    D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
    D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
    
    D3DXMatrixLookAtLH(&matView,
        &position, // 카메라 위치 
        &target, // 시선 위치 
        &up); // 위쪽 방향 

    // 투영 행렬 생성 
    D3DXMatrixPerspectiveFovLH(&matProjection,
        (FLOAT)D3DXToRadian(45), // 시야 
        (FLOAT)SCREEN_WIDTH / (FLOAT)SCREEN_HEIGHT, // 종횡비 
        1.0f, // 가까운 뷰 평면 
        100.0f); // 먼 뷰 평면 

    // 최종 변환 생성 
    matFinal = matRotate * matView * matProjection;

    // 백 버퍼를 진한 파란색으로 지웁니다. 
    devcon->ClearRenderTargetView(backbuffer, D3DXCOLOR(0.0f, 0.2f, 0.4f, 1.0f));

    // 깊이 버퍼를 지웁니다. 
    devcon->ClearDepthStencilView(zbuffer, D3D11_CLEAR_DEPTH, 1.0f, 0);

    // 표시할 정점 버퍼를 선택합니다. 
    UINT stride = sizeof(VERTEX);
    UINT offset = 0;
    devcon->IASetVertexBuffers(0, 1, &pVBuffer, &stride, &offset);
    devcon->IASetIndexBuffer(pIBuffer, DXGI_FORMAT_R32_UINT, 0);

    // 사용할 기본 유형을 선택합니다. 
    devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Hypercraft 그리기 
    devcon->UpdateSubresource(pCBuffer, 0, 0, &matFinal, 0, 0);
    devcon->DrawIndexed(24, 0, 0);

    // 백 버퍼와 프런트 버퍼 전환 
    swapchain->Present(0, 0);
}


// 이 함수는 Direct3D와 COM을 정리합니다 
void CleanD3D(void)
{
    swapchain->SetFullscreenState(FALSE, NULL);    // switch to windowed mode

    // close and release all existing COM objects
    pLayout->Release();
    pVS->Release();
    pPS->Release();
    zbuffer->Release();
    pVBuffer->Release();
    pCBuffer->Release();
    pIBuffer->Release();
    swapchain->Release();
    backbuffer->Release();
    dev->Release();
    devcon->Release();
}


// 이것은 렌더링할 모양을 만드는 함수입니다 
void InitGraphics()
{
    // Hypercraft의 모서리를 나타내는 정점을 만듭니다 
    VERTEX OurVertices[] =
    {
        // 동체 
        {3.0f, 0.0f, 0.0f, D3DXCOLOR(0.0f, 1.0f, 0.0f, 1.0f)},
        {0.0f, 3.0f, -3.0f, D3DXCOLOR(0.0f, 0.0f, 1.0f, 1.0f)},
        {0.0f, 0.0f, 10.0f, D3DXCOLOR(1.0f, 0.0f, 0.0f, 1.0f)},
        {-3.0f, 0.0f, 0.0f, D3DXCOLOR(0.0f, 1.0f, 1.0f, 1.0f)},

        // 왼쪽 총 
        {3.2f, -1.0f, -3.0f, D3DXCOLOR(0.0f, 0.0f, 1.0f, 1.0f)},
        {3.2f, -1.0f, 11.0f, D3DXCOLOR(0.0f, 1.0f, 0.0f, 1.0f)},
        {2.0f, 1.0f, 2.0f, D3DXCOLOR(0.0f, 1.0f, 1.0f, 1.0f)},

        // 오른쪽 총 
        {-3.2f, -1.0f, -3.0f, D3DXCOLOR(0.0f, 0.0f, 1.0f, 1.0f)},
        {-3.2f, -1.0f, 11.0f, D3DXCOLOR(0.0f, 1.0f, 0.0f, 1.0f)},
        {-2.0f, 1.0f, 2.0f, D3DXCOLOR(0.0f, 1.0f, 1.0f, 1.0f)},
    };

    // 정점 버퍼를 생성합니다 
    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof(VERTEX) * 10;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    dev->CreateBuffer(&bd, NULL, &pVBuffer);

    // 정점을 버퍼 
    D3D11_MAPPED_SUBRESOURCE ms;
        devcon->Map(pVBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms); // 버퍼를 매핑합니다 
    memcpy(ms.pData, OurVertices, sizeof(OurVertices)); // 데이터를 복사합니다 
    devcon->Unmap(pVBuffer, NULL);


    // DWORD에서 인덱스 버퍼를 만듭니다 
    DWORD OurIndices[] =
    {
        0, 1, 2, // 동체 
        2, 1, 3,
        3, 1, 0,
        0, 2, 3,
        4, 5, 6, // 날개 
        7, 8, 9,
        4, 6, 5, // 날개(뒷면) 
        7, 9, 8,
    };

    // 인덱스 버퍼를 만듭니다 
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof(DWORD) * 24;
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bd.MiscFlags = 0;

    dev->CreateBuffer(&bd, NULL, &pIBuffer);

    devcon->Map(pIBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms); // 버퍼 매핑 
    memcpy(ms.pData, OurIndices, sizeof(OurIndices)); // 데이터 복사 
    devcon->Unmap(pIBuffer, NULL);
}


// 이 함수는 셰이더를 로드하고 준비합니다. 
void InitPipeline()
{
    // 두 셰이더를 로드하고 컴파일합니다. 
    ID3D10Blob* VS, * PS;
    D3DX11CompileFromFile(L"../shaders.shader", 0, 0, "VShader", "vs_4_0", 0, 0, 0, &VS, 0, 0);
    D3DX11CompileFromFile(L"../shaders.shader", 0, 0, "PShader", "ps_4_0", 0, 0, 0, &PS, 0, 0);

    // 두 셰이더를 셰이더 객체로 캡슐화 
    dev->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &pVS);
    dev->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &pPS);

    // 셰이더 객체를 설정 
    devcon->VSSetShader(pVS, 0, 0);
    devcon->PSSetShader(pPS, 0, 0);

    // 입력 레이아웃 개체 
    D3D11_INPUT_ELEMENT_DESC ied[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    dev->CreateInputLayout(ied, 2, VS->GetBufferPointer(), VS->GetBufferSize(), &pLayout);
    devcon->IASetInputLayout(pLayout);

    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = 64;
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;


    dev->CreateBuffer(&bd, NULL, &pCBuffer);
    devcon->VSSetConstantBuffers(0, 1, &pCBuffer);
}
