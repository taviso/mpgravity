

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0500 */
/* at Sat Sep 11 23:49:35 2010
 */
/* Compiler settings for .\NEWS.odl:
    Oicf, W1, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__


#ifndef __NEWS_i_h__
#define __NEWS_i_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IGravity_FWD_DEFINED__
#define __IGravity_FWD_DEFINED__
typedef interface IGravity IGravity;
#endif 	/* __IGravity_FWD_DEFINED__ */


#ifndef __IGravityNewsgroup_FWD_DEFINED__
#define __IGravityNewsgroup_FWD_DEFINED__
typedef interface IGravityNewsgroup IGravityNewsgroup;
#endif 	/* __IGravityNewsgroup_FWD_DEFINED__ */


#ifndef __CNewsDoc_FWD_DEFINED__
#define __CNewsDoc_FWD_DEFINED__

#ifdef __cplusplus
typedef class CNewsDoc CNewsDoc;
#else
typedef struct CNewsDoc CNewsDoc;
#endif /* __cplusplus */

#endif 	/* __CNewsDoc_FWD_DEFINED__ */


#ifndef __GravityNewsgroup_FWD_DEFINED__
#define __GravityNewsgroup_FWD_DEFINED__

#ifdef __cplusplus
typedef class GravityNewsgroup GravityNewsgroup;
#else
typedef struct GravityNewsgroup GravityNewsgroup;
#endif /* __cplusplus */

#endif 	/* __GravityNewsgroup_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"

#ifdef __cplusplus
extern "C"{
#endif 



#ifndef __Gravity_LIBRARY_DEFINED__
#define __Gravity_LIBRARY_DEFINED__

/* library Gravity */
/* [version][uuid] */ 


EXTERN_C const IID LIBID_Gravity;

#ifndef __IGravity_DISPINTERFACE_DEFINED__
#define __IGravity_DISPINTERFACE_DEFINED__

/* dispinterface IGravity */
/* [uuid] */ 


EXTERN_C const IID DIID_IGravity;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("ABF68602-5A1B-11CE-A7E5-444553540000")
    IGravity : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct IGravityVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IGravity * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IGravity * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IGravity * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IGravity * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IGravity * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IGravity * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IGravity * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        END_INTERFACE
    } IGravityVtbl;

    interface IGravity
    {
        CONST_VTBL struct IGravityVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGravity_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IGravity_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IGravity_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IGravity_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IGravity_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IGravity_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IGravity_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* __IGravity_DISPINTERFACE_DEFINED__ */


#ifndef __IGravityNewsgroup_INTERFACE_DEFINED__
#define __IGravityNewsgroup_INTERFACE_DEFINED__

/* interface IGravityNewsgroup */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IGravityNewsgroup;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("BAA92455-AC7F-4C1A-AA22-9CDA0A8EED06")
    IGravityNewsgroup : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetArticleNumbers( 
            /* [out] */ VARIANT *pVar) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsArticleProtected( 
            /* [in] */ long lArticleNum) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ArticleSetProtected( 
            /* [in] */ long lArtNum,
            /* [in] */ long fProtected) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetArticleByNumber( 
            /* [in] */ long lArtNum,
            /* [out] */ BSTR *pbstrArticle) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Open( 
            /* [in] */ BSTR bsNewsgroup) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IGravityNewsgroupVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IGravityNewsgroup * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IGravityNewsgroup * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IGravityNewsgroup * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IGravityNewsgroup * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IGravityNewsgroup * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IGravityNewsgroup * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IGravityNewsgroup * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetArticleNumbers )( 
            IGravityNewsgroup * This,
            /* [out] */ VARIANT *pVar);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IsArticleProtected )( 
            IGravityNewsgroup * This,
            /* [in] */ long lArticleNum);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *ArticleSetProtected )( 
            IGravityNewsgroup * This,
            /* [in] */ long lArtNum,
            /* [in] */ long fProtected);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetArticleByNumber )( 
            IGravityNewsgroup * This,
            /* [in] */ long lArtNum,
            /* [out] */ BSTR *pbstrArticle);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Open )( 
            IGravityNewsgroup * This,
            /* [in] */ BSTR bsNewsgroup);
        
        END_INTERFACE
    } IGravityNewsgroupVtbl;

    interface IGravityNewsgroup
    {
        CONST_VTBL struct IGravityNewsgroupVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGravityNewsgroup_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IGravityNewsgroup_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IGravityNewsgroup_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IGravityNewsgroup_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IGravityNewsgroup_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IGravityNewsgroup_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IGravityNewsgroup_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IGravityNewsgroup_GetArticleNumbers(This,pVar)	\
    ( (This)->lpVtbl -> GetArticleNumbers(This,pVar) ) 

#define IGravityNewsgroup_IsArticleProtected(This,lArticleNum)	\
    ( (This)->lpVtbl -> IsArticleProtected(This,lArticleNum) ) 

#define IGravityNewsgroup_ArticleSetProtected(This,lArtNum,fProtected)	\
    ( (This)->lpVtbl -> ArticleSetProtected(This,lArtNum,fProtected) ) 

#define IGravityNewsgroup_GetArticleByNumber(This,lArtNum,pbstrArticle)	\
    ( (This)->lpVtbl -> GetArticleByNumber(This,lArtNum,pbstrArticle) ) 

#define IGravityNewsgroup_Open(This,bsNewsgroup)	\
    ( (This)->lpVtbl -> Open(This,bsNewsgroup) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IGravityNewsgroup_INTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_CNewsDoc;

#ifdef __cplusplus

class DECLSPEC_UUID("ABF68600-5A1B-11CE-A7E5-444553540000")
CNewsDoc;
#endif

EXTERN_C const CLSID CLSID_GravityNewsgroup;

#ifdef __cplusplus

class DECLSPEC_UUID("F480A25B-2176-45C5-973C-1A1A83E8F426")
GravityNewsgroup;
#endif
#endif /* __Gravity_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


