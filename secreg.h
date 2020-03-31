/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: secreg.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:49  richard_wood
/*  Updated for VS 2005
/*
/*                                                                           */
/*****************************************************************************/

/**********************************************************************************
Copyright (c) 2003, Albert M. Choy
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.
    * Neither the name of Microplanet, Inc. nor the names of its contributors
      may be used to endorse or promote products derived from this software
      without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
**********************************************************************************/

// Stingray Software Extension Classes
// Copyright (C) 1995 Stingray Software Inc,
// All rights reserved
//           
// CHANGELOG:
// 7/15/95 : Original Implementation based on wfc by Samuel Blackburn
// 7/20/95 : extended for WIN16 APIs
// 9/30/95 : Code review MSW
//

#pragma once
 
#ifdef WIN32
#ifndef _WINREG_
#include "winreg.h"
#endif
#endif

#ifdef _SECDLL
 #undef AFXAPP_DATA
 #define AFXAPP_DATA AFXAPI_DATA
 #undef AFX_DATA
 #define AFX_DATA SEC_DATAEXT
#endif

// defines for Win16 compatibility and convenience
#define THIS_SUB_KEY FALSE
#define ALL_SUB_KEYS TRUE

#define SIGNAL_EVENT    TRUE
#define WAIT_FOR_CHANGE FALSE

#ifndef WIN32
#define ERROR_INVALID_HANDLE 100
#endif

#ifndef WIN32
#define SECPBYTE LPTSTR
#else
#define SECPBYTE LPBYTE
#endif

class SECRegistry : public CObject
{
    DECLARE_DYNAMIC( SECRegistry );

// Enumerations for SECRegistry
public:
#ifdef WIN32
    static HKEY hKeyLocalMachine;
    static HKEY hKeyClassesRoot;
    static HKEY hKeyUsers;
    static HKEY hKeyCurrentUser;
    
    enum CreationDisposition
    {
	dispositionCreatedNewKey     = REG_CREATED_NEW_KEY,
	dispositionOpenedExistingKey = REG_OPENED_EXISTING_KEY
    };
    
    enum CreateOptions
    {
	optionsNonVolatile = REG_OPTION_NON_VOLATILE,
	optionsVolatile    = REG_OPTION_VOLATILE
    };
    
    enum CreatePermissions
    {
	permissionAllAccess        = KEY_ALL_ACCESS,
	permissionCreateLink       = KEY_CREATE_LINK,
	permissionCreateSubKey     = KEY_CREATE_SUB_KEY,
	permissionEnumerateSubKeys = KEY_ENUMERATE_SUB_KEYS,
	permissionExecute          = KEY_EXECUTE,
	permissionNotify           = KEY_NOTIFY,
	permissionQueryValue       = KEY_QUERY_VALUE,
	permissionRead             = KEY_READ,
	permissionSetValue         = KEY_SET_VALUE,
	permissionWrite            = KEY_WRITE
    };

    enum KeyValueTypes
    {
	typeBinary                 = REG_BINARY,
	typeDoubleWord             = REG_DWORD,
	typeDoubleWordLittleEndian = REG_DWORD_LITTLE_ENDIAN,
	typeDoubleWordBigEndian    = REG_DWORD_BIG_ENDIAN,
	typeUnexpandedString       = REG_EXPAND_SZ,
	typeSymbolicLink           = REG_LINK,
	typeMultipleString         = REG_MULTI_SZ,
	typeNone                   = REG_NONE,
	typeResourceList           = REG_RESOURCE_LIST,
	typeString                 = REG_SZ
    };
    
    enum NotifyChangeFilter
    {
	notifyName       = REG_NOTIFY_CHANGE_NAME,
	notifyAttributes = REG_NOTIFY_CHANGE_ATTRIBUTES,
	notifyLastSet    = REG_NOTIFY_CHANGE_LAST_SET,
	notifySecurity   = REG_NOTIFY_CHANGE_SECURITY
    };
    
    enum NotifyChangeFlag
    {
	changeKeyAndSubkeys    = TRUE,
	changeSpecifiedKeyOnly = FALSE
    };
    
#else // WIN16 only supports string key type.
    enum KeyValueTypes
    {
	typeString                 = REG_SZ
    };
    
#endif
	
// Construction
public:
    SECRegistry();
    void Initialize( void );
#ifdef WIN32
    virtual BOOL Create( LPCTSTR               lpszSubkeyName,
			 LPCTSTR               name_of_class         = NULL,
			 CreateOptions         options               = optionsNonVolatile,
			 CreatePermissions     permissions           = permissionAllAccess,
			 LPSECURITY_ATTRIBUTES pSecurityAttributes   = NULL,
			 CreationDisposition * pDisposition          = NULL 
			 );
#else
    virtual BOOL Create( LPCTSTR               name_of_subkey,
			 LPCTSTR               name_of_class         = NULL
			 );
#endif
    
// Attributes and enumerations for SECRegistry
    
    
// Operations
public:

#ifdef WIN32
    virtual BOOL Close( void );
    
    virtual BOOL Connect( HKEY    hKeyToOpen   = HKEY_CURRENT_USER,
			  LPCTSTR lpszComputerName = NULL );
    
    virtual BOOL DeleteKey( LPCTSTR lpszKeyToDelete);
    
    virtual BOOL DeleteValue( LPCTSTR lpszValueToDelete);
    
    virtual BOOL EnumerateKeys( const DWORD dwSubkeyIndex,
				CString&    strSubkeyName,
				CString&    strClassName );
    
    virtual BOOL EnumerateValues( const DWORD    dwValueIndex,
				  CString&       strValueName,
				  KeyValueTypes& type_code,
				  LPBYTE         lpbDataBuffer,
				  DWORD&         dwSizeDataBuffer );
    
    virtual BOOL Flush( void );
    
    virtual BOOL GetBinaryValue( LPCTSTR lpszValueName, CByteArray& return_array );
    
    virtual BOOL GetDoubleWordValue( LPCTSTR lpszValueName, DWORD& dwReturnValue );
    
    virtual BOOL GetSecurity( const SECURITY_INFORMATION security_info,
			      PSECURITY_DESCRIPTOR       data_buffer,
			      DWORD&                     dwSizeDataBuffer);
    
    virtual BOOL GetStringValue( LPCTSTR lpszValueName,
				 CString& strReturn );
    
    virtual BOOL GetStringArrayValue( LPCTSTR name_of_value,
				      CStringArray& return_array );

    virtual BOOL GetValue( LPCTSTR lpszValueName,
			   CByteArray& return_array );
    virtual BOOL GetValue( LPCTSTR lpszValueName,
			   DWORD& dwReturnValue );
    virtual BOOL GetValue( LPCTSTR lpszValueName,
			   CStringArray& return_array );
    virtual BOOL GetValue( LPCTSTR lpszValueName,
			   CString& strReturn );
    
    virtual BOOL Load( LPCTSTR lpszSubkeyName,
		       LPCTSTR lpszFileName );
    
    virtual BOOL NotifyChange( const HANDLE hEvent        = NULL,
			       const NotifyChangeFilter changes_to_be_reported = notifyLastSet,
			       const BOOL bAllSubkeys     = changeSpecifiedKeyOnly,
			       const BOOL bWaitForChange  = WAIT_FOR_CHANGE );
    
    virtual BOOL Open( LPCTSTR lpszSubkey,
		       const CreatePermissions security_access_mask = permissionAllAccess );
    
    virtual BOOL QueryInfo( void );
    virtual BOOL QueryValue( LPCTSTR        lpszValueName,
			     KeyValueTypes& value_type,
			     LPBYTE        lpbBuffer,
			     DWORD&         dwBufferSize);
    
    virtual BOOL Replace( LPCTSTR lpszSubkeyName,
			  LPCTSTR lpszNewFile,
			  LPCTSTR lpszBackupFile);
    
    virtual BOOL Restore( LPCTSTR lpszSavedTreeFile,
			  const DWORD dwVolatilityFlags = NULL );
    
    virtual BOOL Save( LPCTSTR lpszDestFile,
		       LPSECURITY_ATTRIBUTES pSecurityAttributes = NULL );
    
    virtual BOOL SetBinaryValue( LPCTSTR lpszValueName,
				 const CByteArray& bytes_to_write );
    
    virtual BOOL SetDoubleWordValue( LPCTSTR lpszValueName,
				     DWORD dwValue );
    
    virtual BOOL SetSecurity( const SECURITY_INFORMATION& SecurityInformation,
			      const PSECURITY_DESCRIPTOR pSecurityDescriptor );
    
    virtual BOOL SetStringValue( LPCTSTR lpszValueName,
				 const CString& string_value );
    
    virtual BOOL SetStringArrayValue( LPCTSTR lpszValueName,
				      const CStringArray& string_array );
    
    virtual BOOL SetValue( LPCTSTR lpszValueName,
			   const CByteArray& bytes_to_write );
    virtual BOOL SetValue( LPCTSTR lpszValueName, DWORD dwValue );
    virtual BOOL SetValue( LPCTSTR lpszValueName,
			   const CStringArray& strings_to_write );
    virtual BOOL SetValue( LPCTSTR lpszValueName,
			   const CString& strWrite );
    
    virtual BOOL SetValue( LPCTSTR             lpszValueName,
			   const KeyValueTypes type_of_value_to_set,
			   LPBYTE              lpbValueData,
			   const DWORD         dwSize );
    
    virtual BOOL UnLoad( LPCTSTR lpszSubkey );
#else // WIN16
    virtual BOOL Close( void );
    
    virtual BOOL DeleteKey( LPCTSTR lpszKeyToDelete );
    
    virtual BOOL EnumerateKeys( const DWORD dwIndex,
				LPTSTR       lpszBuffer,
				DWORD       dwBufferSize);
	
	virtual BOOL GetSubkeys(LPCTSTR lpszBuffer, UINT& nKeys);	
    
    virtual BOOL Open( LPCTSTR lpszSubkey);
    
    virtual BOOL QueryValue( LPCTSTR  lpSubKey,
			     SECPBYTE lpbBuffer,
			     LONG&    lBufferSize);
    
    virtual BOOL SetValue( LPCTSTR lpSubKey,
			   const CString& strWrite );
#endif

// Implementation    
public:
    virtual ~SECRegistry();
    
    LONG m_lErrorCode;
    HKEY m_hKey;
    HKEY m_hRegistry;
    
#ifdef WIN32
    DWORD    m_dwLongestSubkeyNameLength;
    DWORD    m_dwLongestClassNameLength;
    DWORD    m_dwLongestValueNameLength;
    DWORD    m_dwLongestValueDataLength;
    DWORD    m_dwSecurityDescriptorLength;
    FILETIME m_fileTimeLastWrite;
#endif

    CString m_strClassName;
    CString m_strComputerName;
    CString m_strKeyName;
    CString m_strRegistryName;
    DWORD   m_dwNumberOfSubkeys;
    DWORD   m_dwNumberOfValues;
    CTime   m_timeLastWrite;
};

//
// SEC Extension DLL
// Reset declaration context
//
#undef AFX_DATA
#define AFX_DATA
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR
