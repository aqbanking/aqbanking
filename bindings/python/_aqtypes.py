# -*- encoding: utf-8 -*-
# This file is generated be aqcodegen
# DO NOT CHANGE BY HAND

from _basetypes import *
from enum import Enum, check_enum


class BankInfoService(c_void_p):

    def _check_retval_(p):
        if p is None:
            return None
        v = BankInfoService.__new__(BankInfoService)
        c_void_p.__init__(v, p)
        aqb.AB_BankInfoService_Attach(v)
        return v
    _check_retval_ = staticmethod(_check_retval_)

    def __init__(self):
        tr = aqb.AB_BankInfoService_new()
        c_void_p.__init__(self, tr)

    def __del__(self):
        aqb.AB_BankInfoService_free(self)

    modified = property(
        aqb.AB_BankInfoService_IsModified,
        aqb.AB_BankInfoService_SetModified)

    type = property(
        aqb.AB_BankInfoService_GetType,
        aqb.AB_BankInfoService_SetType,
        'The following types have been registered with AqBanking:\n'
        'HBCI - German homebanking protocol\n'
        'OFX  - OFX direct connect protocol')

    address = property(
        aqb.AB_BankInfoService_GetAddress,
        aqb.AB_BankInfoService_SetAddress,
        'For most services this is the URL or hostname of the server.')

    suffix = property(
        aqb.AB_BankInfoService_GetSuffix,
        aqb.AB_BankInfoService_SetSuffix,
        'For IP based services this is the port to be used (if omitted\n'
        'a default value suitable for the service is chosen).')

    pversion = property(
        aqb.AB_BankInfoService_GetPversion,
        aqb.AB_BankInfoService_SetPversion,
        'The content of this field depends on the service type.\n'
        'For HBCI this is the protocol version to be used:\n'
        '2.01\n'
        '2.10\n'
        '2.20')

    mode = property(
        aqb.AB_BankInfoService_GetMode,
        aqb.AB_BankInfoService_SetMode,
        'The content of this field depends on the service type.\n'
        'For HBCI the following values are used:\n'
        'DDV\n'
        'RDH1\n'
        'RDH2\n'
        'RDH3\n'
        'RDH4\n'
        'PINTAN')

    aux1 = property(
        aqb.AB_BankInfoService_GetAux1,
        aqb.AB_BankInfoService_SetAux1,
        'This is a multi purpose field to be used by a bankinfo plugin as\n'
        'it sees fit.\n'
        'OFX uses this to store the FID from the bankinfo file.')

    aux2 = property(
        aqb.AB_BankInfoService_GetAux2,
        aqb.AB_BankInfoService_SetAux2,
        'This is a multi purpose field to be used by a bankinfo plugin as\n'
        'it sees fit.\n'
        'OFX uses this to store the ORG field from the bankinfo file.')

    def __str__(self):
        return "<class BankInfoService:\nmodified=%s\ntype=%s\naddress=%s\nsuffix=%s\npversion=%s\nmode=%s\naux1=%s\naux2=%s\n/BankInfoService>" % (self.modified,self.type,self.address,self.suffix,self.pversion,self.mode,self.aux1,self.aux2)


class BankInfo(c_void_p):

    def _check_retval_(p):
        if p is None:
            return None
        v = BankInfo.__new__(BankInfo)
        c_void_p.__init__(v, p)
        aqb.AB_BankInfo_Attach(v)
        return v
    _check_retval_ = staticmethod(_check_retval_)

    def __init__(self):
        tr = aqb.AB_BankInfo_new()
        c_void_p.__init__(self, tr)

    def __del__(self):
        aqb.AB_BankInfo_free(self)

    modified = property(
        aqb.AB_BankInfo_IsModified,
        aqb.AB_BankInfo_SetModified)

    country = property(
        aqb.AB_BankInfo_GetCountry,
        aqb.AB_BankInfo_SetCountry)

    branchId = property(
        aqb.AB_BankInfo_GetBranchId,
        aqb.AB_BankInfo_SetBranchId)

    bankId = property(
        aqb.AB_BankInfo_GetBankId,
        aqb.AB_BankInfo_SetBankId)

    bic = property(
        aqb.AB_BankInfo_GetBic,
        aqb.AB_BankInfo_SetBic)

    bankName = property(
        aqb.AB_BankInfo_GetBankName,
        aqb.AB_BankInfo_SetBankName)

    location = property(
        aqb.AB_BankInfo_GetLocation,
        aqb.AB_BankInfo_SetLocation)

    street = property(
        aqb.AB_BankInfo_GetStreet,
        aqb.AB_BankInfo_SetStreet)

    zipcode = property(
        aqb.AB_BankInfo_GetZipcode,
        aqb.AB_BankInfo_SetZipcode)

    city = property(
        aqb.AB_BankInfo_GetCity,
        aqb.AB_BankInfo_SetCity)

    region = property(
        aqb.AB_BankInfo_GetRegion,
        aqb.AB_BankInfo_SetRegion)

    phone = property(
        aqb.AB_BankInfo_GetPhone,
        aqb.AB_BankInfo_SetPhone)

    fax = property(
        aqb.AB_BankInfo_GetFax,
        aqb.AB_BankInfo_SetFax)

    email = property(
        aqb.AB_BankInfo_GetEmail,
        aqb.AB_BankInfo_SetEmail)

    website = property(
        aqb.AB_BankInfo_GetWebsite,
        aqb.AB_BankInfo_SetWebsite)

    services = property(
        aqb.AB_BankInfo_GetServices,
        lambda self, v: aqb.AB_BankInfo_SetServices(self, makeBankInfoServicelist(v)))

    def __str__(self):
        return "<class BankInfo:\nmodified=%s\ncountry=%s\nbranchId=%s\nbankId=%s\nbic=%s\nbankName=%s\nlocation=%s\nstreet=%s\nzipcode=%s\ncity=%s\nregion=%s\nphone=%s\nfax=%s\nemail=%s\nwebsite=%s\nservices=%s\n/BankInfo>" % (self.modified,self.country,self.branchId,self.bankId,self.bic,self.bankName,self.location,self.street,self.zipcode,self.city,self.region,self.phone,self.fax,self.email,self.website,self.services)


class EuTransferInfo(c_void_p):

    def _check_retval_(p):
        if p is None:
            return None
        v = EuTransferInfo.__new__(EuTransferInfo)
        c_void_p.__init__(v, p)
        aqb.AB_EuTransferInfo_Attach(v)
        return v
    _check_retval_ = staticmethod(_check_retval_)

    def __init__(self):
        tr = aqb.AB_EuTransferInfo_new()
        c_void_p.__init__(self, tr)

    def __del__(self):
        aqb.AB_EuTransferInfo_free(self)

    modified = property(
        aqb.AB_EuTransferInfo_IsModified,
        aqb.AB_EuTransferInfo_SetModified)

    countryCode = property(
        aqb.AB_EuTransferInfo_GetCountryCode,
        aqb.AB_EuTransferInfo_SetCountryCode,
        'This is the two-character ISO country code (as used in toplevel\n'
        'domains). For Germany use "DE".')

    fieldLimits = property(
        aqb.AB_EuTransferInfo_GetFieldLimits,
        lambda self, v: aqb.AB_EuTransferInfo_SetFieldLimits(self, makeTransactionLimitslist(v)))

    limitLocalValue = property(
        aqb.AB_EuTransferInfo_GetLimitLocalValue,
        aqb.AB_EuTransferInfo_SetLimitLocalValue,
        'Optional limit for a transfer in local currency.')

    limitForeignValue = property(
        aqb.AB_EuTransferInfo_GetLimitForeignValue,
        aqb.AB_EuTransferInfo_SetLimitForeignValue,
        'Optional limit for a transfer in foreign currency.')

    def __str__(self):
        return "<class EuTransferInfo:\nmodified=%s\ncountryCode=%s\nfieldLimits=%s\nlimitLocalValue=%s\nlimitForeignValue=%s\n/EuTransferInfo>" % (self.modified,self.countryCode,self.fieldLimits,self.limitLocalValue,self.limitForeignValue)


class Pin(c_void_p):

    def _check_retval_(p):
        if p is None:
            return None
        v = Pin.__new__(Pin)
        c_void_p.__init__(v, p)
        aqb.AB_Pin_Attach(v)
        return v
    _check_retval_ = staticmethod(_check_retval_)

    def __init__(self):
        tr = aqb.AB_Pin_new()
        c_void_p.__init__(self, tr)

    def __del__(self):
        aqb.AB_Pin_free(self)

    modified = property(
        aqb.AB_Pin_IsModified,
        aqb.AB_Pin_SetModified)

    token = property(
        aqb.AB_Pin_GetToken,
        aqb.AB_Pin_SetToken)

    value = property(
        aqb.AB_Pin_GetValue,
        aqb.AB_Pin_SetValue)

    hash = property(
        aqb.AB_Pin_GetHash,
        aqb.AB_Pin_SetHash)

    status = property(
        aqb.AB_Pin_GetStatus,
        aqb.AB_Pin_SetStatus)

    def __str__(self):
        return "<class Pin:\nmodified=%s\ntoken=%s\nvalue=%s\nhash=%s\nstatus=%s\n/Pin>" % (self.modified,self.token,self.value,self.hash,self.status)


class Split(c_void_p):
    'This type contains all important information about transaction splits.\n'
    'Please note that all text fields are in UTF-8.'

    def _check_retval_(p):
        if p is None:
            return None
        v = Split.__new__(Split)
        c_void_p.__init__(v, p)
        aqb.AB_Split_Attach(v)
        return v
    _check_retval_ = staticmethod(_check_retval_)

    def __init__(self):
        tr = aqb.AB_Split_new()
        c_void_p.__init__(self, tr)

    def __del__(self):
        aqb.AB_Split_free(self)

    modified = property(
        aqb.AB_Split_IsModified,
        aqb.AB_Split_SetModified)

    # Group Account Info
    # This group contains information about the remote account.
    # This is the two-character ISO country code (as used in toplevel
    # domains). For Germany use "DE".
    # This is the branch id of the remote bank (OFX only)

    country = property(
        aqb.AB_Split_GetCountry,
        aqb.AB_Split_SetCountry,
        'This is the two-character ISO country code (as used in toplevel\n'
        'domains). For Germany use "DE".')

    bankCode = property(
        aqb.AB_Split_GetBankCode,
        aqb.AB_Split_SetBankCode)

    branchId = property(
        aqb.AB_Split_GetBranchId,
        aqb.AB_Split_SetBranchId,
        'This is the branch id of the remote bank (OFX only)')

    accountNumber = property(
        aqb.AB_Split_GetAccountNumber,
        aqb.AB_Split_SetAccountNumber)

    suffix = property(
        aqb.AB_Split_GetSuffix,
        aqb.AB_Split_SetSuffix)

    name = property(
        aqb.AB_Split_GetName,
        lambda self, v: aqb.AB_Split_SetName(self, makeStringlist(v)))

    # Group Value

    value = property(
        aqb.AB_Split_GetValue,
        aqb.AB_Split_SetValue)

    purpose = property(
        aqb.AB_Split_GetPurpose,
        lambda self, v: aqb.AB_Split_SetPurpose(self, makeStringlist(v)))

    def __str__(self):
        return "<class Split:\nmodified=%s\ncountry=%s\nbankCode=%s\nbranchId=%s\naccountNumber=%s\nsuffix=%s\nname=%s\nvalue=%s\npurpose=%s\n/Split>" % (self.modified,self.country,self.bankCode,self.branchId,self.accountNumber,self.suffix,self.name,self.value,self.purpose)


class TransactionLimits(c_void_p):
    'This type describes the limits for fields of an @ref AB_TRANSACTION.\n'
    'The limits have the following meanings:\n'
    'maxLenSOMETHING: if 0 then this limit is unknown, if -1 then the\n'
    'described element is not allowed to be set in the transaction.\n'
    'All other values represent the maximum lenght of the described\n'
    'field.\n'
    'minLenSOMETHING: if 0 then this limit is unknown.\n'
    'All other values represent the minimum lenght of the described\n'
    'field.\n'
    'maxLinesSOMETHING: if 0 then this limit is unknown\n'
    'All other values represent the maximum number of lines for the\n'
    'described field.\n'
    'minLinesSOMETHING: if 0 then this limit is unknown.\n'
    'All other values represent the minimum number of lines for the\n'
    'described field.\n'
    'valuesSOMETHING: A list of allowed values (as string). If this list\n'
    'is empty then there all values are allowed (those lists @b exist in\n'
    'any case, so the appropriate getter function will never return NULL).\n'
    'So if you want to check whether an given field is at all allowed you\n'
    'must check whether "maxLenSOMETHING" has a value of "-1".'

    def _check_retval_(p):
        if p is None:
            return None
        v = TransactionLimits.__new__(TransactionLimits)
        c_void_p.__init__(v, p)
        aqb.AB_TransactionLimits_Attach(v)
        return v
    _check_retval_ = staticmethod(_check_retval_)

    def __init__(self):
        tr = aqb.AB_TransactionLimits_new()
        c_void_p.__init__(self, tr)

    def __del__(self):
        aqb.AB_TransactionLimits_free(self)

    modified = property(
        aqb.AB_TransactionLimits_IsModified,
        aqb.AB_TransactionLimits_SetModified)

    # Group Issuer Name
    # Limits for the issuer name.

    maxLenLocalName = property(
        aqb.AB_TransactionLimits_GetMaxLenLocalName,
        aqb.AB_TransactionLimits_SetMaxLenLocalName)

    minLenLocalName = property(
        aqb.AB_TransactionLimits_GetMinLenLocalName,
        aqb.AB_TransactionLimits_SetMinLenLocalName)

    # Group Payee Name
    # Limits for the payee name.

    maxLenRemoteName = property(
        aqb.AB_TransactionLimits_GetMaxLenRemoteName,
        aqb.AB_TransactionLimits_SetMaxLenRemoteName)

    minLenRemoteName = property(
        aqb.AB_TransactionLimits_GetMinLenRemoteName,
        aqb.AB_TransactionLimits_SetMinLenRemoteName)

    maxLinesRemoteName = property(
        aqb.AB_TransactionLimits_GetMaxLinesRemoteName,
        aqb.AB_TransactionLimits_SetMaxLinesRemoteName)

    minLinesRemoteName = property(
        aqb.AB_TransactionLimits_GetMinLinesRemoteName,
        aqb.AB_TransactionLimits_SetMinLinesRemoteName)

    # Group Local Bank Code
    # Limits for local bank code.

    maxLenLocalBankCode = property(
        aqb.AB_TransactionLimits_GetMaxLenLocalBankCode,
        aqb.AB_TransactionLimits_SetMaxLenLocalBankCode)

    minLenLocalBankCode = property(
        aqb.AB_TransactionLimits_GetMinLenLocalBankCode,
        aqb.AB_TransactionLimits_SetMinLenLocalBankCode)

    # Group Local Account Id
    # Limits for local account id.

    maxLenLocalAccountNumber = property(
        aqb.AB_TransactionLimits_GetMaxLenLocalAccountNumber,
        aqb.AB_TransactionLimits_SetMaxLenLocalAccountNumber)

    minLenLocalAccountNumber = property(
        aqb.AB_TransactionLimits_GetMinLenLocalAccountNumber,
        aqb.AB_TransactionLimits_SetMinLenLocalAccountNumber)

    # Group Local Account Number
    # Limits for local account id suffix.

    maxLenLocalSuffix = property(
        aqb.AB_TransactionLimits_GetMaxLenLocalSuffix,
        aqb.AB_TransactionLimits_SetMaxLenLocalSuffix)

    minLenLocalSuffix = property(
        aqb.AB_TransactionLimits_GetMinLenLocalSuffix,
        aqb.AB_TransactionLimits_SetMinLenLocalSuffix)

    # Group Remote Bank Code
    # Limits for remote bank code.

    maxLenRemoteBankCode = property(
        aqb.AB_TransactionLimits_GetMaxLenRemoteBankCode,
        aqb.AB_TransactionLimits_SetMaxLenRemoteBankCode)

    minLenRemoteBankCode = property(
        aqb.AB_TransactionLimits_GetMinLenRemoteBankCode,
        aqb.AB_TransactionLimits_SetMinLenRemoteBankCode)

    # Group Remote Account Number
    # Limits for remote account number.

    maxLenRemoteAccountNumber = property(
        aqb.AB_TransactionLimits_GetMaxLenRemoteAccountNumber,
        aqb.AB_TransactionLimits_SetMaxLenRemoteAccountNumber)

    minLenRemoteAccountNumber = property(
        aqb.AB_TransactionLimits_GetMinLenRemoteAccountNumber,
        aqb.AB_TransactionLimits_SetMinLenRemoteAccountNumber)

    # Group Remote Account Number Suffix
    # Limits for remote account id suffix.

    maxLenRemoteSuffix = property(
        aqb.AB_TransactionLimits_GetMaxLenRemoteSuffix,
        aqb.AB_TransactionLimits_SetMaxLenRemoteSuffix)

    minLenRemoteSuffix = property(
        aqb.AB_TransactionLimits_GetMinLenRemoteSuffix,
        aqb.AB_TransactionLimits_SetMinLenRemoteSuffix)

    # Group Remote IBAN
    # Limits for remote IAN.

    maxLenRemoteIban = property(
        aqb.AB_TransactionLimits_GetMaxLenRemoteIban,
        aqb.AB_TransactionLimits_SetMaxLenRemoteIban)

    minLenRemoteIban = property(
        aqb.AB_TransactionLimits_GetMinLenRemoteIban,
        aqb.AB_TransactionLimits_SetMinLenRemoteIban)

    # Group Text Key
    # Limits for textKey.
    # This string list contains one entry for every supported text key.
    # The values must be positive integers in decimal form (no leading
    # zero, no comma or decimal point).

    maxLenTextKey = property(
        aqb.AB_TransactionLimits_GetMaxLenTextKey,
        aqb.AB_TransactionLimits_SetMaxLenTextKey)

    minLenTextKey = property(
        aqb.AB_TransactionLimits_GetMinLenTextKey,
        aqb.AB_TransactionLimits_SetMinLenTextKey)

    valuesTextKey = property(
        aqb.AB_TransactionLimits_GetValuesTextKey,
        lambda self, v: aqb.AB_TransactionLimits_SetValuesTextKey(self, makeStringlist(v)))

    # Group Customer Reference
    # Limits for customer reference.

    maxLenCustomerReference = property(
        aqb.AB_TransactionLimits_GetMaxLenCustomerReference,
        aqb.AB_TransactionLimits_SetMaxLenCustomerReference)

    minLenCustomerReference = property(
        aqb.AB_TransactionLimits_GetMinLenCustomerReference,
        aqb.AB_TransactionLimits_SetMinLenCustomerReference)

    # Group Bank Reference
    # Limits for bank reference.

    maxLenBankReference = property(
        aqb.AB_TransactionLimits_GetMaxLenBankReference,
        aqb.AB_TransactionLimits_SetMaxLenBankReference)

    minLenBankReference = property(
        aqb.AB_TransactionLimits_GetMinLenBankReference,
        aqb.AB_TransactionLimits_SetMinLenBankReference)

    # Group Purpose
    # Limits for purpose (called memo in some apps).

    maxLenPurpose = property(
        aqb.AB_TransactionLimits_GetMaxLenPurpose,
        aqb.AB_TransactionLimits_SetMaxLenPurpose)

    minLenPurpose = property(
        aqb.AB_TransactionLimits_GetMinLenPurpose,
        aqb.AB_TransactionLimits_SetMinLenPurpose)

    maxLinesPurpose = property(
        aqb.AB_TransactionLimits_GetMaxLinesPurpose,
        aqb.AB_TransactionLimits_SetMaxLinesPurpose)

    minLinesPurpose = property(
        aqb.AB_TransactionLimits_GetMinLinesPurpose,
        aqb.AB_TransactionLimits_SetMinLinesPurpose)

    def __str__(self):
        return "<class TransactionLimits:\nmodified=%s\nmaxLenLocalName=%s\nminLenLocalName=%s\nmaxLenRemoteName=%s\nminLenRemoteName=%s\nmaxLinesRemoteName=%s\nminLinesRemoteName=%s\nmaxLenLocalBankCode=%s\nminLenLocalBankCode=%s\nmaxLenLocalAccountNumber=%s\nminLenLocalAccountNumber=%s\nmaxLenLocalSuffix=%s\nminLenLocalSuffix=%s\nmaxLenRemoteBankCode=%s\nminLenRemoteBankCode=%s\nmaxLenRemoteAccountNumber=%s\nminLenRemoteAccountNumber=%s\nmaxLenRemoteSuffix=%s\nminLenRemoteSuffix=%s\nmaxLenRemoteIban=%s\nminLenRemoteIban=%s\nmaxLenTextKey=%s\nminLenTextKey=%s\nvaluesTextKey=%s\nmaxLenCustomerReference=%s\nminLenCustomerReference=%s\nmaxLenBankReference=%s\nminLenBankReference=%s\nmaxLenPurpose=%s\nminLenPurpose=%s\nmaxLinesPurpose=%s\nminLinesPurpose=%s\n/TransactionLimits>" % (self.modified,self.maxLenLocalName,self.minLenLocalName,self.maxLenRemoteName,self.minLenRemoteName,self.maxLinesRemoteName,self.minLinesRemoteName,self.maxLenLocalBankCode,self.minLenLocalBankCode,self.maxLenLocalAccountNumber,self.minLenLocalAccountNumber,self.maxLenLocalSuffix,self.minLenLocalSuffix,self.maxLenRemoteBankCode,self.minLenRemoteBankCode,self.maxLenRemoteAccountNumber,self.minLenRemoteAccountNumber,self.maxLenRemoteSuffix,self.minLenRemoteSuffix,self.maxLenRemoteIban,self.minLenRemoteIban,self.maxLenTextKey,self.minLenTextKey,self.valuesTextKey,self.maxLenCustomerReference,self.minLenCustomerReference,self.maxLenBankReference,self.minLenBankReference,self.maxLenPurpose,self.minLenPurpose,self.maxLinesPurpose,self.minLinesPurpose)


class Transaction(c_void_p):
    'This type contains all important information about transactions.\n'
    'All text fields are in UTF-8.\n'
    'Please note: Since version 0.9.9.1 of AqBanking a transaction may\n'
    'contain splits.\n'
    'If an AB_TRANSACTION actually does contain splits then some variables\n'
    '(like localCountry) are stored within the AB_SPLITs rather than\n'
    'in AB_TRANSACTION.\n'
    'So your application should first check for splits and read the\n'
    'information (marked as in AB_SPLIT below) from them.'

    def _check_retval_(p):
        if p is None:
            return None
        v = Transaction.__new__(Transaction)
        c_void_p.__init__(v, p)
        aqb.AB_Transaction_Attach(v)
        return v
    _check_retval_ = staticmethod(_check_retval_)

    def __init__(self):
        tr = aqb.AB_Transaction_new()
        c_void_p.__init__(self, tr)

    def __del__(self):
        aqb.AB_Transaction_free(self)

    class Period(Enum):
        monthly = 0 # The standing order is to be executed every month.
        weekly = 1 # The standing order is to be executed every week.

    class PeriodAdapter(c_int):
        def _check_retval_(i):
            return Period(i)
        _check_retval_ = staticmethod(_check_retval_)
        def from_param(cls, e):
            check_enum(e, Period, 'argument')
            return int(e)
        from_param = classmethod(from_param)

    modified = property(
        aqb.AB_Transaction_IsModified,
        aqb.AB_Transaction_SetModified)

    # Group Local Account Info
    # This group contains information about the local account.
    # Functions of this group are also available in AB_SPLIT, please
    # make your application check for splits first and use the values here
    # as a fallback.
    # This is the two-character country code according to ISO
    # 3166-1 (Alpha-2). This is also used in DNS toplevel domain
    # names. For Germany use "DE" (not case-sensitive).
    # This is the code of the local bank (i.e. your bank).
    # This is the branch id of the local bank (OFX only)
    # If your account has subaccounts which are distinguished by
    # different suffixes, then this is that suffix. Otherwise it's
    # empty. (HBCI only)

    localCountry = property(
        aqb.AB_Transaction_GetLocalCountry,
        aqb.AB_Transaction_SetLocalCountry,
        'This is the two-character country code according to ISO\n'
        '3166-1 (Alpha-2). This is also used in DNS toplevel domain\n'
        'names. For Germany use "DE" (not case-sensitive).')

    localBankCode = property(
        aqb.AB_Transaction_GetLocalBankCode,
        aqb.AB_Transaction_SetLocalBankCode,
        'This is the code of the local bank (i.e. your bank).')

    localBranchId = property(
        aqb.AB_Transaction_GetLocalBranchId,
        aqb.AB_Transaction_SetLocalBranchId,
        'This is the branch id of the local bank (OFX only)')

    localAccountNumber = property(
        aqb.AB_Transaction_GetLocalAccountNumber,
        aqb.AB_Transaction_SetLocalAccountNumber)

    localSuffix = property(
        aqb.AB_Transaction_GetLocalSuffix,
        aqb.AB_Transaction_SetLocalSuffix,
        'If your account has subaccounts which are distinguished by\n'
        'different suffixes, then this is that suffix. Otherwise it\'s\n'
        'empty. (HBCI only)')

    localName = property(
        aqb.AB_Transaction_GetLocalName,
        aqb.AB_Transaction_SetLocalName)

    # Group Remote Account Info
    # This group contains information about the remote account.
    # Functions of this group are also available in AB_SPLIT, please
    # make your application check for splits first and use the values here
    # as a fallback.
    # This is the two-character ISO country code (as used in toplevel
    # domains). For Germany use "DE".
    # This is the branch id of the remote bank (OFX only)
    # International Bank Account Number according to ECBS EBS 204.
    # PosMeaning
    # 0-1Country code according to ISO 3166
    # 2-3Checksum
    # 4-33Country specific account info
    # Examples:
    # BE62510007547061
    # FR1420041010050500013M02606

    remoteCountry = property(
        aqb.AB_Transaction_GetRemoteCountry,
        aqb.AB_Transaction_SetRemoteCountry,
        'This is the two-character ISO country code (as used in toplevel\n'
        'domains). For Germany use "DE".')

    remoteBankName = property(
        aqb.AB_Transaction_GetRemoteBankName,
        aqb.AB_Transaction_SetRemoteBankName)

    remoteBankLocation = property(
        aqb.AB_Transaction_GetRemoteBankLocation,
        aqb.AB_Transaction_SetRemoteBankLocation)

    remoteBankCode = property(
        aqb.AB_Transaction_GetRemoteBankCode,
        aqb.AB_Transaction_SetRemoteBankCode)

    remoteBranchId = property(
        aqb.AB_Transaction_GetRemoteBranchId,
        aqb.AB_Transaction_SetRemoteBranchId,
        'This is the branch id of the remote bank (OFX only)')

    remoteAccountNumber = property(
        aqb.AB_Transaction_GetRemoteAccountNumber,
        aqb.AB_Transaction_SetRemoteAccountNumber)

    remoteSuffix = property(
        aqb.AB_Transaction_GetRemoteSuffix,
        aqb.AB_Transaction_SetRemoteSuffix)

    remoteIban = property(
        aqb.AB_Transaction_GetRemoteIban,
        aqb.AB_Transaction_SetRemoteIban,
        'International Bank Account Number according to ECBS EBS 204.\n'
        'PosMeaning\n'
        '0-1Country code according to ISO 3166\n'
        '2-3Checksum\n'
        '4-33Country specific account info\n'
        'Examples:\n'
        'BE62510007547061\n'
        'FR1420041010050500013M02606')

    remoteName = property(
        aqb.AB_Transaction_GetRemoteName,
        lambda self, v: aqb.AB_Transaction_SetRemoteName(self, makeStringlist(v)))

    uniqueId = property(
        aqb.AB_Transaction_GetUniqueId,
        aqb.AB_Transaction_SetUniqueId,
        'This is a unique id assigned by the application. However, when\n'
        'adding a transaction to a job (like JobTransfer) this id is\n'
        'assigned by AqBanking to make sure that this id is unique across\n'
        'all applications.')

    # Group Dates
    # The date when the transaction was really executed
    # (Datum Valuta/Wertstellung)
    # The date when the transaction was booked (but sometimes it is
    # unused). (Buchungsdatum)

    valutaDate = property(
        aqb.AB_Transaction_GetValutaDate,
        aqb.AB_Transaction_SetValutaDate,
        'The date when the transaction was really executed\n'
        '(Datum Valuta/Wertstellung)')

    date = property(
        aqb.AB_Transaction_GetDate,
        aqb.AB_Transaction_SetDate,
        'The date when the transaction was booked (but sometimes it is\n'
        'unused). (Buchungsdatum)')

    # Group Value
    # Functions of this group are also available in AB_SPLIT, please
    # make your application check for splits first and use the values here
    # as a fallback.

    value = property(
        aqb.AB_Transaction_GetValue,
        aqb.AB_Transaction_SetValue)

    splits = property(
        aqb.AB_Transaction_GetSplits,
        lambda self, v: aqb.AB_Transaction_SetSplits(self, makeSplitlist(v)))

    # Group Info Which Is Not Supported by All Backends
    # This group contains information which differ between backends.
    # Some of this information might not even be supported by every
    # backends.
    # A 3 digit numerical transaction code, defined for all kinds of
    # different actions. (Textschluessel)
    # For a normal transfer you should set it to 51. For debit notes
    # the values 04 or 05 may be used. For other values please refer to
    # your credit institute. (HBCI only)
    # this is the transaction id that tells you more about the type
    # of transaction (3 character code) (Buchungsschluessel)
    # (HBCI only)
    # Reference string, if the customer (you) has specified
    # one. (E.g. the cheque number.) Otherwise "NONREF" or empty
    # (Kundenreferenz)
    # Reference string for this transaction given by the bank, if it
    # has given one. Otherwise empty. (Bankreferenz)
    # A 3 digit numerical transaction code, defined for all kinds of
    # different actions. (Geschaeftsvorfallcode)
    # Transaction text (e.g. STANDING ORDER) (Buchungstext)
    # This is an id optionally assigned by the Financial
    # Institute. It is mostly used by OFX.
    # This string list contains the purpose of the transaction.
    # Every entry of this list represents a single purpose line.
    # This string list contains the categories this transaction
    # belongs to. This element is not used by AqBanking itself but
    # some im/exporter plugins may choose to use these.

    textKey = property(
        aqb.AB_Transaction_GetTextKey,
        aqb.AB_Transaction_SetTextKey,
        'A 3 digit numerical transaction code, defined for all kinds of\n'
        'different actions. (Textschluessel)\n'
        'For a normal transfer you should set it to 51. For debit notes\n'
        'the values 04 or 05 may be used. For other values please refer to\n'
        'your credit institute. (HBCI only)')

    transactionKey = property(
        aqb.AB_Transaction_GetTransactionKey,
        aqb.AB_Transaction_SetTransactionKey,
        'this is the transaction id that tells you more about the type\n'
        'of transaction (3 character code) (Buchungsschluessel)\n'
        '(HBCI only)')

    customerReference = property(
        aqb.AB_Transaction_GetCustomerReference,
        aqb.AB_Transaction_SetCustomerReference,
        'Reference string, if the customer (you) has specified\n'
        'one. (E.g. the cheque number.) Otherwise "NONREF" or empty\n'
        '(Kundenreferenz)')

    bankReference = property(
        aqb.AB_Transaction_GetBankReference,
        aqb.AB_Transaction_SetBankReference,
        'Reference string for this transaction given by the bank, if it\n'
        'has given one. Otherwise empty. (Bankreferenz)')

    transactionCode = property(
        aqb.AB_Transaction_GetTransactionCode,
        aqb.AB_Transaction_SetTransactionCode,
        'A 3 digit numerical transaction code, defined for all kinds of\n'
        'different actions. (Geschaeftsvorfallcode)')

    transactionText = property(
        aqb.AB_Transaction_GetTransactionText,
        aqb.AB_Transaction_SetTransactionText,
        'Transaction text (e.g. STANDING ORDER) (Buchungstext)')

    primanota = property(
        aqb.AB_Transaction_GetPrimanota,
        aqb.AB_Transaction_SetPrimanota)

    fiId = property(
        aqb.AB_Transaction_GetFiId,
        aqb.AB_Transaction_SetFiId,
        'This is an id optionally assigned by the Financial\n'
        'Institute. It is mostly used by OFX.')

    purpose = property(
        aqb.AB_Transaction_GetPurpose,
        lambda self, v: aqb.AB_Transaction_SetPurpose(self, makeStringlist(v)))

    category = property(
        aqb.AB_Transaction_GetCategory,
        lambda self, v: aqb.AB_Transaction_SetCategory(self, makeStringlist(v)))

    # Group Additional Information for Standing Orders
    # This group contains information which is used with standing orders.
    # It is not needed for other usage of this type.
    # This variable contains the execution period (e.g. whether a standing
    # order is to be executed weekly or monthly etc).
    # The standing order is executed every cycle x period.
    # So if period is weekly and cycle is 2
    # then the standing order is executed every 2 weeks.
    # The execution day. The meaning of this variable depends on the
    # content of period:
    # monthly: day of the month (starting with 1)
    # weekly: day of the week (starting with 1=Monday)
    # The date when the standing order is to be executed for the first
    # time.
    # The date when the standing order is to be executed for the last
    # time.
    # The date when the standing order is to be executed next (this field
    # is only interesting when retrieving the list of currently active
    # standing orders)

    period = property(
        aqb.AB_Transaction_GetPeriod,
        aqb.AB_Transaction_SetPeriod,
        'This variable contains the execution period (e.g. whether a standing\n'
        'order is to be executed weekly or monthly etc).')

    cycle = property(
        aqb.AB_Transaction_GetCycle,
        aqb.AB_Transaction_SetCycle,
        'The standing order is executed every cycle x period.\n'
        'So if period is weekly and cycle is 2\n'
        'then the standing order is executed every 2 weeks.')

    executionDay = property(
        aqb.AB_Transaction_GetExecutionDay,
        aqb.AB_Transaction_SetExecutionDay,
        'The execution day. The meaning of this variable depends on the\n'
        'content of period:\n'
        'monthly: day of the month (starting with 1)\n'
        'weekly: day of the week (starting with 1=Monday)')

    firstExecutionDate = property(
        aqb.AB_Transaction_GetFirstExecutionDate,
        aqb.AB_Transaction_SetFirstExecutionDate,
        'The date when the standing order is to be executed for the first\n'
        'time.')

    lastExecutionDate = property(
        aqb.AB_Transaction_GetLastExecutionDate,
        aqb.AB_Transaction_SetLastExecutionDate,
        'The date when the standing order is to be executed for the last\n'
        'time.')

    nextExecutionDate = property(
        aqb.AB_Transaction_GetNextExecutionDate,
        aqb.AB_Transaction_SetNextExecutionDate,
        'The date when the standing order is to be executed next (this field\n'
        'is only interesting when retrieving the list of currently active\n'
        'standing orders)')

    def __str__(self):
        return "<class Transaction:\nmodified=%s\nlocalCountry=%s\nlocalBankCode=%s\nlocalBranchId=%s\nlocalAccountNumber=%s\nlocalSuffix=%s\nlocalName=%s\nremoteCountry=%s\nremoteBankName=%s\nremoteBankLocation=%s\nremoteBankCode=%s\nremoteBranchId=%s\nremoteAccountNumber=%s\nremoteSuffix=%s\nremoteIban=%s\nremoteName=%s\nuniqueId=%s\nvalutaDate=%s\ndate=%s\nvalue=%s\nsplits=%s\ntextKey=%s\ntransactionKey=%s\ncustomerReference=%s\nbankReference=%s\ntransactionCode=%s\ntransactionText=%s\nprimanota=%s\nfiId=%s\npurpose=%s\ncategory=%s\nperiod=%s\ncycle=%s\nexecutionDay=%s\nfirstExecutionDate=%s\nlastExecutionDate=%s\nnextExecutionDate=%s\n/Transaction>" % (self.modified,self.localCountry,self.localBankCode,self.localBranchId,self.localAccountNumber,self.localSuffix,self.localName,self.remoteCountry,self.remoteBankName,self.remoteBankLocation,self.remoteBankCode,self.remoteBranchId,self.remoteAccountNumber,self.remoteSuffix,self.remoteIban,self.remoteName,self.uniqueId,self.valutaDate,self.date,self.value,self.splits,self.textKey,self.transactionKey,self.customerReference,self.bankReference,self.transactionCode,self.transactionText,self.primanota,self.fiId,self.purpose,self.category,self.period,self.cycle,self.executionDay,self.firstExecutionDate,self.lastExecutionDate,self.nextExecutionDate)


# list helpers

def makeStringlist(v):
    if v is None:
        return None
    sl = gwen.GWEN_StringList_new()
    for s in v:
        se = gwen.GWEN_StringListEntry_new(s, False)
        gwen.GWEN_StringList_AppendEntry(sl, se)
    return sl

def tupleStringlist(sl):
    if not sl:
        return ()
    l = []
    e = gwen.GWEN_StringList_FirstEntry(sl)
    while e:
        l.append(gwen.GWEN_StringListEntry_Data(e))
        e = gwen.GWEN_StringListEntry_Next(e)
    return tuple(l)

gwen.GWEN_StringListEntry_new.argtypes = c_char_p, c_int
gwen.GWEN_StringListEntry_Data.restype = c_char_p

def makeBankInfoServicelist(v):
    if v is None:
        return None
    sl = aqb.AB_BankInfoService_List_new()
    for s in v:
        aqb.AB_BankInfoService_List_Add(s, c_void_p(sl))
    return sl

def tupleBankInfoServicelist(sl):
    if not sl:
        return ()
    e = aqb.AB_BankInfoService_List_First(sl)
    l = []
    while e:
        l.append(BankInfoService._check_retval_(e))
        e = aqb.AB_BankInfoService_List_Next(e)
    return tuple(l)

aqb.AB_BankInfoService_List_Add.argtypes = BankInfoService, c_void_p

def makeSplitlist(v):
    if v is None:
        return None
    sl = aqb.AB_Split_List_new()
    for s in v:
        aqb.AB_Split_List_Add(s, c_void_p(sl))
    return sl

def tupleSplitlist(sl):
    if not sl:
        return ()
    e = aqb.AB_Split_List_First(sl)
    l = []
    while e:
        l.append(Split._check_retval_(e))
        e = aqb.AB_Split_List_Next(e)
    return tuple(l)

aqb.AB_Split_List_Add.argtypes = Split, c_void_p

def makeTransactionLimitslist(v):
    if v is None:
        return None
    sl = aqb.AB_TransactionLimits_List_new()
    for s in v:
        aqb.AB_TransactionLimits_List_Add(s, c_void_p(sl))
    return sl

def tupleTransactionLimitslist(sl):
    if not sl:
        return ()
    e = aqb.AB_TransactionLimits_List_First(sl)
    l = []
    while e:
        l.append(TransactionLimits._check_retval_(e))
        e = aqb.AB_TransactionLimits_List_Next(e)
    return tuple(l)

aqb.AB_TransactionLimits_List_Add.argtypes = TransactionLimits, c_void_p


# Pin
aqb.AB_Pin_IsModified.restype = c_int
aqb.AB_Pin_SetModified.argtypes = Pin, c_int
aqb.AB_Pin_GetToken.restype = c_char_p
aqb.AB_Pin_SetToken.argtypes = Pin, c_char_p
aqb.AB_Pin_GetValue.restype = c_char_p
aqb.AB_Pin_SetValue.argtypes = Pin, c_char_p
aqb.AB_Pin_GetHash.restype = c_char_p
aqb.AB_Pin_SetHash.argtypes = Pin, c_char_p
aqb.AB_Pin_GetStatus.restype = c_char_p
aqb.AB_Pin_SetStatus.argtypes = Pin, c_char_p

# EuTransferInfo
aqb.AB_EuTransferInfo_IsModified.restype = c_int
aqb.AB_EuTransferInfo_SetModified.argtypes = EuTransferInfo, c_int
aqb.AB_EuTransferInfo_GetCountryCode.restype = c_char_p
aqb.AB_EuTransferInfo_SetCountryCode.argtypes = EuTransferInfo, c_char_p
aqb.AB_EuTransferInfo_GetFieldLimits.restype = tupleTransactionLimitslist
aqb.AB_EuTransferInfo_GetLimitLocalValue.restype = Value
aqb.AB_EuTransferInfo_SetLimitLocalValue.argtypes = EuTransferInfo, Value
aqb.AB_EuTransferInfo_GetLimitForeignValue.restype = Value
aqb.AB_EuTransferInfo_SetLimitForeignValue.argtypes = EuTransferInfo, Value

# BankInfo
aqb.AB_BankInfo_IsModified.restype = c_int
aqb.AB_BankInfo_SetModified.argtypes = BankInfo, c_int
aqb.AB_BankInfo_GetCountry.restype = c_char_p
aqb.AB_BankInfo_SetCountry.argtypes = BankInfo, c_char_p
aqb.AB_BankInfo_GetBranchId.restype = c_char_p
aqb.AB_BankInfo_SetBranchId.argtypes = BankInfo, c_char_p
aqb.AB_BankInfo_GetBankId.restype = c_char_p
aqb.AB_BankInfo_SetBankId.argtypes = BankInfo, c_char_p
aqb.AB_BankInfo_GetBic.restype = c_char_p
aqb.AB_BankInfo_SetBic.argtypes = BankInfo, c_char_p
aqb.AB_BankInfo_GetBankName.restype = c_char_p
aqb.AB_BankInfo_SetBankName.argtypes = BankInfo, c_char_p
aqb.AB_BankInfo_GetLocation.restype = c_char_p
aqb.AB_BankInfo_SetLocation.argtypes = BankInfo, c_char_p
aqb.AB_BankInfo_GetStreet.restype = c_char_p
aqb.AB_BankInfo_SetStreet.argtypes = BankInfo, c_char_p
aqb.AB_BankInfo_GetZipcode.restype = c_char_p
aqb.AB_BankInfo_SetZipcode.argtypes = BankInfo, c_char_p
aqb.AB_BankInfo_GetCity.restype = c_char_p
aqb.AB_BankInfo_SetCity.argtypes = BankInfo, c_char_p
aqb.AB_BankInfo_GetRegion.restype = c_char_p
aqb.AB_BankInfo_SetRegion.argtypes = BankInfo, c_char_p
aqb.AB_BankInfo_GetPhone.restype = c_char_p
aqb.AB_BankInfo_SetPhone.argtypes = BankInfo, c_char_p
aqb.AB_BankInfo_GetFax.restype = c_char_p
aqb.AB_BankInfo_SetFax.argtypes = BankInfo, c_char_p
aqb.AB_BankInfo_GetEmail.restype = c_char_p
aqb.AB_BankInfo_SetEmail.argtypes = BankInfo, c_char_p
aqb.AB_BankInfo_GetWebsite.restype = c_char_p
aqb.AB_BankInfo_SetWebsite.argtypes = BankInfo, c_char_p
aqb.AB_BankInfo_GetServices.restype = tupleBankInfoServicelist

# Transaction
aqb.AB_Transaction_IsModified.restype = c_int
aqb.AB_Transaction_SetModified.argtypes = Transaction, c_int
aqb.AB_Transaction_GetLocalCountry.restype = c_char_p
aqb.AB_Transaction_SetLocalCountry.argtypes = Transaction, c_char_p
aqb.AB_Transaction_GetLocalBankCode.restype = c_char_p
aqb.AB_Transaction_SetLocalBankCode.argtypes = Transaction, c_char_p
aqb.AB_Transaction_GetLocalBranchId.restype = c_char_p
aqb.AB_Transaction_SetLocalBranchId.argtypes = Transaction, c_char_p
aqb.AB_Transaction_GetLocalAccountNumber.restype = c_char_p
aqb.AB_Transaction_SetLocalAccountNumber.argtypes = Transaction, c_char_p
aqb.AB_Transaction_GetLocalSuffix.restype = c_char_p
aqb.AB_Transaction_SetLocalSuffix.argtypes = Transaction, c_char_p
aqb.AB_Transaction_GetLocalName.restype = c_char_p
aqb.AB_Transaction_SetLocalName.argtypes = Transaction, c_char_p
aqb.AB_Transaction_GetRemoteCountry.restype = c_char_p
aqb.AB_Transaction_SetRemoteCountry.argtypes = Transaction, c_char_p
aqb.AB_Transaction_GetRemoteBankName.restype = c_char_p
aqb.AB_Transaction_SetRemoteBankName.argtypes = Transaction, c_char_p
aqb.AB_Transaction_GetRemoteBankLocation.restype = c_char_p
aqb.AB_Transaction_SetRemoteBankLocation.argtypes = Transaction, c_char_p
aqb.AB_Transaction_GetRemoteBankCode.restype = c_char_p
aqb.AB_Transaction_SetRemoteBankCode.argtypes = Transaction, c_char_p
aqb.AB_Transaction_GetRemoteBranchId.restype = c_char_p
aqb.AB_Transaction_SetRemoteBranchId.argtypes = Transaction, c_char_p
aqb.AB_Transaction_GetRemoteAccountNumber.restype = c_char_p
aqb.AB_Transaction_SetRemoteAccountNumber.argtypes = Transaction, c_char_p
aqb.AB_Transaction_GetRemoteSuffix.restype = c_char_p
aqb.AB_Transaction_SetRemoteSuffix.argtypes = Transaction, c_char_p
aqb.AB_Transaction_GetRemoteIban.restype = c_char_p
aqb.AB_Transaction_SetRemoteIban.argtypes = Transaction, c_char_p
aqb.AB_Transaction_GetRemoteName.restype = tupleStringlist
aqb.AB_Transaction_GetUniqueId.restype = c_int
aqb.AB_Transaction_SetUniqueId.argtypes = Transaction, c_int
aqb.AB_Transaction_GetValutaDate.restype = GWEN_Time
aqb.AB_Transaction_SetValutaDate.argtypes = Transaction, GWEN_Time
aqb.AB_Transaction_GetDate.restype = GWEN_Time
aqb.AB_Transaction_SetDate.argtypes = Transaction, GWEN_Time
aqb.AB_Transaction_GetValue.restype = Value
aqb.AB_Transaction_SetValue.argtypes = Transaction, Value
aqb.AB_Transaction_GetSplits.restype = tupleSplitlist
aqb.AB_Transaction_GetTextKey.restype = c_int
aqb.AB_Transaction_SetTextKey.argtypes = Transaction, c_int
aqb.AB_Transaction_GetTransactionKey.restype = c_char_p
aqb.AB_Transaction_SetTransactionKey.argtypes = Transaction, c_char_p
aqb.AB_Transaction_GetCustomerReference.restype = c_char_p
aqb.AB_Transaction_SetCustomerReference.argtypes = Transaction, c_char_p
aqb.AB_Transaction_GetBankReference.restype = c_char_p
aqb.AB_Transaction_SetBankReference.argtypes = Transaction, c_char_p
aqb.AB_Transaction_GetTransactionCode.restype = c_int
aqb.AB_Transaction_SetTransactionCode.argtypes = Transaction, c_int
aqb.AB_Transaction_GetTransactionText.restype = c_char_p
aqb.AB_Transaction_SetTransactionText.argtypes = Transaction, c_char_p
aqb.AB_Transaction_GetPrimanota.restype = c_char_p
aqb.AB_Transaction_SetPrimanota.argtypes = Transaction, c_char_p
aqb.AB_Transaction_GetFiId.restype = c_char_p
aqb.AB_Transaction_SetFiId.argtypes = Transaction, c_char_p
aqb.AB_Transaction_GetPurpose.restype = tupleStringlist
aqb.AB_Transaction_GetCategory.restype = tupleStringlist
aqb.AB_Transaction_GetPeriod.restype = Transaction.PeriodAdapter
aqb.AB_Transaction_SetPeriod.argtypes = Transaction, Transaction.PeriodAdapter
aqb.AB_Transaction_GetCycle.restype = c_int
aqb.AB_Transaction_SetCycle.argtypes = Transaction, c_int
aqb.AB_Transaction_GetExecutionDay.restype = c_int
aqb.AB_Transaction_SetExecutionDay.argtypes = Transaction, c_int
aqb.AB_Transaction_GetFirstExecutionDate.restype = GWEN_Time
aqb.AB_Transaction_SetFirstExecutionDate.argtypes = Transaction, GWEN_Time
aqb.AB_Transaction_GetLastExecutionDate.restype = GWEN_Time
aqb.AB_Transaction_SetLastExecutionDate.argtypes = Transaction, GWEN_Time
aqb.AB_Transaction_GetNextExecutionDate.restype = GWEN_Time
aqb.AB_Transaction_SetNextExecutionDate.argtypes = Transaction, GWEN_Time

# Split
aqb.AB_Split_IsModified.restype = c_int
aqb.AB_Split_SetModified.argtypes = Split, c_int
aqb.AB_Split_GetCountry.restype = c_char_p
aqb.AB_Split_SetCountry.argtypes = Split, c_char_p
aqb.AB_Split_GetBankCode.restype = c_char_p
aqb.AB_Split_SetBankCode.argtypes = Split, c_char_p
aqb.AB_Split_GetBranchId.restype = c_char_p
aqb.AB_Split_SetBranchId.argtypes = Split, c_char_p
aqb.AB_Split_GetAccountNumber.restype = c_char_p
aqb.AB_Split_SetAccountNumber.argtypes = Split, c_char_p
aqb.AB_Split_GetSuffix.restype = c_char_p
aqb.AB_Split_SetSuffix.argtypes = Split, c_char_p
aqb.AB_Split_GetName.restype = tupleStringlist
aqb.AB_Split_GetValue.restype = Value
aqb.AB_Split_SetValue.argtypes = Split, Value
aqb.AB_Split_GetPurpose.restype = tupleStringlist

# BankInfoService
aqb.AB_BankInfoService_IsModified.restype = c_int
aqb.AB_BankInfoService_SetModified.argtypes = BankInfoService, c_int
aqb.AB_BankInfoService_GetType.restype = c_char_p
aqb.AB_BankInfoService_SetType.argtypes = BankInfoService, c_char_p
aqb.AB_BankInfoService_GetAddress.restype = c_char_p
aqb.AB_BankInfoService_SetAddress.argtypes = BankInfoService, c_char_p
aqb.AB_BankInfoService_GetSuffix.restype = c_char_p
aqb.AB_BankInfoService_SetSuffix.argtypes = BankInfoService, c_char_p
aqb.AB_BankInfoService_GetPversion.restype = c_char_p
aqb.AB_BankInfoService_SetPversion.argtypes = BankInfoService, c_char_p
aqb.AB_BankInfoService_GetMode.restype = c_char_p
aqb.AB_BankInfoService_SetMode.argtypes = BankInfoService, c_char_p
aqb.AB_BankInfoService_GetAux1.restype = c_char_p
aqb.AB_BankInfoService_SetAux1.argtypes = BankInfoService, c_char_p
aqb.AB_BankInfoService_GetAux2.restype = c_char_p
aqb.AB_BankInfoService_SetAux2.argtypes = BankInfoService, c_char_p

# TransactionLimits
aqb.AB_TransactionLimits_IsModified.restype = c_int
aqb.AB_TransactionLimits_SetModified.argtypes = TransactionLimits, c_int
aqb.AB_TransactionLimits_GetMaxLenLocalName.restype = c_int
aqb.AB_TransactionLimits_SetMaxLenLocalName.argtypes = TransactionLimits, c_int
aqb.AB_TransactionLimits_GetMinLenLocalName.restype = c_int
aqb.AB_TransactionLimits_SetMinLenLocalName.argtypes = TransactionLimits, c_int
aqb.AB_TransactionLimits_GetMaxLenRemoteName.restype = c_int
aqb.AB_TransactionLimits_SetMaxLenRemoteName.argtypes = TransactionLimits, c_int
aqb.AB_TransactionLimits_GetMinLenRemoteName.restype = c_int
aqb.AB_TransactionLimits_SetMinLenRemoteName.argtypes = TransactionLimits, c_int
aqb.AB_TransactionLimits_GetMaxLinesRemoteName.restype = c_int
aqb.AB_TransactionLimits_SetMaxLinesRemoteName.argtypes = TransactionLimits, c_int
aqb.AB_TransactionLimits_GetMinLinesRemoteName.restype = c_int
aqb.AB_TransactionLimits_SetMinLinesRemoteName.argtypes = TransactionLimits, c_int
aqb.AB_TransactionLimits_GetMaxLenLocalBankCode.restype = c_int
aqb.AB_TransactionLimits_SetMaxLenLocalBankCode.argtypes = TransactionLimits, c_int
aqb.AB_TransactionLimits_GetMinLenLocalBankCode.restype = c_int
aqb.AB_TransactionLimits_SetMinLenLocalBankCode.argtypes = TransactionLimits, c_int
aqb.AB_TransactionLimits_GetMaxLenLocalAccountNumber.restype = c_int
aqb.AB_TransactionLimits_SetMaxLenLocalAccountNumber.argtypes = TransactionLimits, c_int
aqb.AB_TransactionLimits_GetMinLenLocalAccountNumber.restype = c_int
aqb.AB_TransactionLimits_SetMinLenLocalAccountNumber.argtypes = TransactionLimits, c_int
aqb.AB_TransactionLimits_GetMaxLenLocalSuffix.restype = c_int
aqb.AB_TransactionLimits_SetMaxLenLocalSuffix.argtypes = TransactionLimits, c_int
aqb.AB_TransactionLimits_GetMinLenLocalSuffix.restype = c_int
aqb.AB_TransactionLimits_SetMinLenLocalSuffix.argtypes = TransactionLimits, c_int
aqb.AB_TransactionLimits_GetMaxLenRemoteBankCode.restype = c_int
aqb.AB_TransactionLimits_SetMaxLenRemoteBankCode.argtypes = TransactionLimits, c_int
aqb.AB_TransactionLimits_GetMinLenRemoteBankCode.restype = c_int
aqb.AB_TransactionLimits_SetMinLenRemoteBankCode.argtypes = TransactionLimits, c_int
aqb.AB_TransactionLimits_GetMaxLenRemoteAccountNumber.restype = c_int
aqb.AB_TransactionLimits_SetMaxLenRemoteAccountNumber.argtypes = TransactionLimits, c_int
aqb.AB_TransactionLimits_GetMinLenRemoteAccountNumber.restype = c_int
aqb.AB_TransactionLimits_SetMinLenRemoteAccountNumber.argtypes = TransactionLimits, c_int
aqb.AB_TransactionLimits_GetMaxLenRemoteSuffix.restype = c_int
aqb.AB_TransactionLimits_SetMaxLenRemoteSuffix.argtypes = TransactionLimits, c_int
aqb.AB_TransactionLimits_GetMinLenRemoteSuffix.restype = c_int
aqb.AB_TransactionLimits_SetMinLenRemoteSuffix.argtypes = TransactionLimits, c_int
aqb.AB_TransactionLimits_GetMaxLenRemoteIban.restype = c_int
aqb.AB_TransactionLimits_SetMaxLenRemoteIban.argtypes = TransactionLimits, c_int
aqb.AB_TransactionLimits_GetMinLenRemoteIban.restype = c_int
aqb.AB_TransactionLimits_SetMinLenRemoteIban.argtypes = TransactionLimits, c_int
aqb.AB_TransactionLimits_GetMaxLenTextKey.restype = c_int
aqb.AB_TransactionLimits_SetMaxLenTextKey.argtypes = TransactionLimits, c_int
aqb.AB_TransactionLimits_GetMinLenTextKey.restype = c_int
aqb.AB_TransactionLimits_SetMinLenTextKey.argtypes = TransactionLimits, c_int
aqb.AB_TransactionLimits_GetValuesTextKey.restype = tupleStringlist
aqb.AB_TransactionLimits_GetMaxLenCustomerReference.restype = c_int
aqb.AB_TransactionLimits_SetMaxLenCustomerReference.argtypes = TransactionLimits, c_int
aqb.AB_TransactionLimits_GetMinLenCustomerReference.restype = c_int
aqb.AB_TransactionLimits_SetMinLenCustomerReference.argtypes = TransactionLimits, c_int
aqb.AB_TransactionLimits_GetMaxLenBankReference.restype = c_int
aqb.AB_TransactionLimits_SetMaxLenBankReference.argtypes = TransactionLimits, c_int
aqb.AB_TransactionLimits_GetMinLenBankReference.restype = c_int
aqb.AB_TransactionLimits_SetMinLenBankReference.argtypes = TransactionLimits, c_int
aqb.AB_TransactionLimits_GetMaxLenPurpose.restype = c_int
aqb.AB_TransactionLimits_SetMaxLenPurpose.argtypes = TransactionLimits, c_int
aqb.AB_TransactionLimits_GetMinLenPurpose.restype = c_int
aqb.AB_TransactionLimits_SetMinLenPurpose.argtypes = TransactionLimits, c_int
aqb.AB_TransactionLimits_GetMaxLinesPurpose.restype = c_int
aqb.AB_TransactionLimits_SetMaxLinesPurpose.argtypes = TransactionLimits, c_int
aqb.AB_TransactionLimits_GetMinLinesPurpose.restype = c_int
aqb.AB_TransactionLimits_SetMinLinesPurpose.argtypes = TransactionLimits, c_int
