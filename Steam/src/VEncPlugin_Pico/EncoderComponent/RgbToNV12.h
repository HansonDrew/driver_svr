#pragma once
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(obj) \
   if (obj != NULL)       \
   {                      \
      obj->Release();     \
      obj = NULL;         \
   }
#endif
class RGBToNV12
{
public:
	RGBToNV12(ID3D11Device *pDev, ID3D11DeviceContext *pCtx);
	~RGBToNV12();
	HRESULT  Init();
	HRESULT  Convert(ID3D11Texture2D* pRGB, ID3D11Texture2D*pYUV);
	void  Cleanup();
	ID3D11Device *m_pDev;
	ID3D11DeviceContext *m_pCtx;
	ID3D11VideoDevice *m_pVid = NULL;
	ID3D11VideoContext *m_pVidCtx = NULL;
	D3D11_TEXTURE2D_DESC m_inDesc;
	D3D11_TEXTURE2D_DESC m_outDesc  ;
	ID3D11VideoProcessor *m_pVP = NULL;
	ID3D11VideoProcessorInputView *pVPIn = NULL;
	ID3D11VideoProcessorOutputView *pVPOV = NULL;
	ID3D11Texture2D *pTexBgra = NULL;
	ID3D11VideoProcessorEnumerator *m_pVPEnum = nullptr;
	std::unordered_map<ID3D11Texture2D*, ID3D11VideoProcessorOutputView*> viewMap;
private:

};
 