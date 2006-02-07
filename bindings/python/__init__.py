# -*- encoding: latin1 -*-
# Copyright (C) 2005 by Andreas Degert
# see LICENSE.txt in the top level distribution directory
#
"""aqbanking -- Interface to the homebanking library aqBanking

This module is a ctypes-based wrapper for aqBanking.

The naming corresponds to aqBanking, but common prefixes are deleted,
case adjusted, and a lot of accessor functions are hidden behind
properties. The complicated list operations of aqBanking are
transformed into iterators. There is a subclass of Job for each job
type.

Subclasses of Enum (defined in module enum) are used to define classes
that represent C enumerations. Calling with an int generates an
instance of EnumInstance, which can display the enum name (or the
corresponding number). Undefined int values raise an exception.

One Example:

The C function AB_JobGetTransactions_new() is used in the constructor of the
python class JobGetTransactions.  AB_JobGetTransactions_GetFromTime() and
AB_JobGetTransactions_SetFromTime() are used to define the getter and setter
of the property fromTime. The list returned by
AB_JobGetTransactions_GetTransactions() is accessable as an iterator returned
by the method iterTransactions().

Classes
  BankingBase           abstract base for the main banking class
                        (cf. console.BankingConsole and gtkui.BankingGtk)
  Job                   abstract base class for jobs
  JobGetBalance         get balance for an account
  JobGetTransactions    get the transactions for an account
  JobSingleTransfer     transfer money from your account to another
  JobSingleDebitNote    retrieve money from another account
  JobEuTransfer         transfer money from your account to another, EU-wide
  Transaction           account numbers etc. for the transfer job
  ImExporterContext     container to hold data for im/export

Enumerations
  JobType               job type (corresponds to Job subclasses)
  JobStatus             status of job execution
  AB_Error              error return codes
  LoggerLevel           levels for aqBanking logging
  LogLevel              levels for messages to the user
  PinStatus             status of the PIN (security code)
  TanStatus             status of the TAN (one time security code)

Exceptions
  BankingError          raised when aqBanking returns an error

Functions
  stripText()           return the text or html part of a message
  msg_flags_type()      type of a message
  msg_flags_confirm_button()
                        number of button signaling a "confirm"
  Logger_SetLevel()     set logging level
  Logger_GetLevel()     get logging level

Classes which must not be instanciated directly:
  Value                 a money/currency value
  GWEN_DB_Node          storage tree node
  AccountInfo           imported data for an account
  Account               local bank account
  AccountStatus         fetched with JobGetBalance
  Balance               Value and time of an account balance
  EuTransferInfo        limiting properties of the euro transfer job
  ImExporter            Importer or Exporter for a specific format
  PluginDescription     describes a plugin (e.g. an ImExporter)
  BufferedIO            aqBanking io abstraction
  
"""
#
# Implementation Notes
#
# The goal is to use ctypes to get a "pythonic" interface to
# aqBanking.
#
# - the structures and concepts represented as aqBanking function
#   groups are mapped to classes
#
# - the accessor functions for field values are used to define
#   properties.
#
# - list operations are encapsulated in iterators.
#
# - error returns from functions raise a BankingError exception.
#
# - ini- / fini-functions are called automatically
#
# - callbacks for interactivity are methods of the main banking class
#
# - referenced C structures are owned by the python object (e.g. by
#   using the attach function of the C interface if one exists), or
#   the python object references the object which keeps the owning C
#   structure (e.g. by defining a property with the helper function
#   property_addref). When the python object is finalized it frees the
#   C structure.
#   FIXME: not yet finished
#
# - bad arguments raise exceptions
#   FIXME: not yet finished
#
# The ctypes tutorial should help understanding its usage in this
# module.
#
from _aqtypes import *
import re
from enum import Enum, check_enum
import atexit, weakref

__all__ = ["Transaction", "Job", "JobSingleDebitNote",
           "JobSingleTransfer", "JobEuTransfer",
           "JobGetTransactions", "JobGetBalance",
           "ImExporterContext", "AB_Error", "BankingError", "JobStatus",
           "LoggerLevel", "LogLevel", "Value", "Balance","Split","AccountInfo",
           "Logger_SetLevel","Logger_GetLevel","PathFlags","DBFlags"]


c_int_p = POINTER(c_int)


################################################################
# Constants, Logging
#
class AB_Error(Enum):
    "error values for C function returns"
    (ok,
     generic,
     not_supported,
     not_available,
     bad_config_file,
     invalid,
     network,
     not_found,
     empty,
     user_abort,
     found,
     no_data,
     nofn,
     unknown_account,
     not_init,
     security,
     bad_data,
     unknown,
     aborted,
     default_value,
     ) = range(0,-20,-1)
    (user1,
     user2,
     user3,
     user4,
     ) = range(-128,-132,-1)


class AccountType(Enum):
    (unknown,
     bank,
     creditcard,
     checking,
     savings,
     investment,
     cash
     ) = range(7)

class AccountTypeAdapter(c_int):
    def _check_retval_(i):
        return AccountType(i)
    _check_retval_ = staticmethod(_check_retval_)
def from_param(cls, e):
    check_enum(e, AccountType, 'argument')
    return int(e)
AccountTypeAdapter.from_param = classmethod(from_param)


class LoggerLevel(Enum):
    "Levels for logging system"
    (emergency,
     alert,
     critical,
     error,
     warning,
     notice,
     info,
     debug,
     verbous,
     ) = range(9)

    unknown = 9999

def Logger_SetLevel(domain, level):
    check_enum(level, LoggerLevel, 'level')
    gwen.GWEN_Logger_SetLevel(domain, level)

gwen.GWEN_Logger_SetLevel.argtypes = c_char_p, c_int

def Logger_GetLevel(domain):
    return gwen.GWEN_Logger_GetLevel(domain)

gwen.GWEN_Logger_GetLevel.restype = LoggerLevel
gwen.GWEN_Logger_GetLevel.argtypes = c_char_p,


################################################################
# exceptions
#
class BankingError(Exception):
    """raised when a C function returns an error.

    error is the AB_Error (if its defined).
    nr is the numeric value.

    When the return value was not in AB_Error,
    error should be set to None (just set nr)
    """
    def __init__(self, text, error, nr):
        self.error = error
        self.nr = nr
        Exception.__init__(self, text)


################################################################
# internal helper functions
#
def chk(ret):
    """Check the return value of a C function.

    For all C functions returning an AB_Error value. Can also be
    used as .restype
    """
    if ret == 0:
	return
    try:
        v = AB_Error(ret)
    except ValueError:
	raise BankingError('unknown error (%d)' % ret, None, ret)
    raise BankingError('%s (%d)' % (v, v), v, ret)


################################################################
# iterators as adapters for functions returning list pointers
#
def iterStringlist(sl):
    "Expects a GWEN_STRINGLIST pointer"
    if not sl:
        return
    e = gwen.GWEN_StringList_FirstEntry(sl)
    while e:
        s = gwen.GWEN_StringListEntry_Data(e)
        if s is None:
            s = ''
        yield s
        e = gwen.GWEN_StringListEntry_Next(e)

def iterJobs(jl):
    "Expects a AB_JOB_LIST2 pointer"
    if not jl:
        return
    jit = aqb.AB_Job_List2_First(jl)
    jp = aqb.AB_Job_List2Iterator_Data(jit)
    while jp:
        yield Job._check_retval_(jp)
        jp = aqb.AB_Job_List2Iterator_Next(jit)
    aqb.AB_Job_List2Iterator_free(jit)
    aqb.AB_Job_List2_free(jl)

################################################################
# adapter classes
#
class GWEN_Buffer(c_void_p):
    """Create a writeable GWEN_BUFFER for .argtypes

    Converting to a string reads the buffer contents:

    p = GWEN_Buffer()
    some_c_func(p)
    print p
    some_python_func(str(p))
    """
    def __init__(self, inisize=200):
        c_void_p.__init__(
            self, gwen.GWEN_Buffer_new(None, inisize, 0, False))

    def __str__(self):
        return c_char_p(gwen.GWEN_Buffer_GetStart(self)).value

    usedBytes = property(gwen.GWEN_Buffer_GetUsedBytes)

    def __del__(self):
        gwen.GWEN_Buffer_free(self)


################################################################
# GWEN_DB
#
class PathFlags:
    PATHMUSTEXIST    = 0x00000001
    PATHMUSTNOTEXIST = 0x00000002
    PATHCREATE       = 0x00000004
    NAMEMUSTEXIST    = 0x00000008
    NAMEMUSTNOTEXIST = 0x00000010
    CREATE_GROUP     = 0x00000020
    CREATE_VAR       = 0x00000040
    VARIABLE         = 0x00000080
    ESCAPE           = 0x00000100
    UNESCAPE         = 0x00000100
    TOLERANT_ESCAPE  = 0x00000200
    CONVERT_LAST     = 0x00000400
    CHECKROOT        = 0x00000800
    NO_IDX           = 0x00001000
    RFU1             = 0x00002000

class DBFlags:
    OVERWRITE_VARS      = 0x00010000
    OVERWRITE_GROUPS    = 0x00020000
    QUOTE_VARNAMES      = 0x00040000
    QUOTE_VALUES        = 0x00080000
    WRITE_SUBGROUPS     = 0x00100000
    DETAILED_GROUPS     = 0x00200000
    INDEND              = 0x00400000
    ADD_GROUP_NEWLINES  = 0x00800000
    USE_COLON           = 0x01000000
    STOP_ON_EMPTY_LINE  = 0x02000000
    OMIT_TYPES          = 0x04000000
    APPEND_FILE         = 0x08000000
    ESCAPE_CHARVALUES   = 0x10000000
    UNESCAPE_CHARVALUES = 0x10000000
    LOCKFILE            = 0x20000000


class GWEN_DB_Node(c_void_p):
    """A node in a storage tree.

    iterGroups()   iterates over the Groups
    """
    VALUETYPE_UNKNOWN = 0
    VALUETYPE_CHAR = 1
    VALUETYPE_INT = 2
    VALUETYPE_BIN = 3
    VALUETYPE_PTR = 4

    def _check_retval_(p):
        if not p:
            return None
        if gwen.GWEN_DB_IsGroup(p):
            cls = GWEN_DB_GroupNode
        elif gwen.GWEN_DB_IsVariable(p):
            cls = GWEN_DB_VariableNode
        elif gwen.GWEN_DB_IsValue(p):
            tp = gwen.GWEN_DB_GetValueType(p)
            cls = GWEN_DB_Node._typetable.get(tp)
            if cls is None:
                raise RuntimeError('unknown GWEN_DB_Node value type %d' % tp)
        v = GWEN_DB_Node.__new__(cls)
        c_void_p.__init__(v, p)
        return v
    _check_retval_ = staticmethod(_check_retval_)

    def __init__(self):
        assert False, "class GWEN_DB_Node is abstract"

    def getGroup(self, flags=0, path=None):
        if path is None:
            raise RuntimeError('please specify a path')
        return gwen.GWEN_DB_GetGroup(self, flags, path)

    def iterGroups(self):
	n = gwen.GWEN_DB_GetFirstGroup(self)
	while n:
	    yield n
	    n = gwen.GWEN_DB_GetNextGroup(n)

    def iterVars(self):
        n = gwen.GWEN_DB_GetFirstVar(self)
        while n:
            yield n
            n = gwen.GWEN_DB_GetNextVar(n)

    def iterValues(self):
        n = gwen.GWEN_DB_GetFirstValue(self)
        while n:
            yield n
            n = gwen.GWEN_DB_GetNextValue(n)

    def getIntValue(self, path='', idx=0, defval=-1):
        return gwen.GWEN_DB_GetIntValue(self, path, idx, defval)

    def __repr__(self):
	return "<%s %s>" % (self.__class__.__name__, self.name)

gwen.GWEN_DB_GetFirstValue.restype = GWEN_DB_Node
gwen.GWEN_DB_GetNextValue.restype = GWEN_DB_Node
gwen.GWEN_DB_GetFirstVar.restype = GWEN_DB_Node
gwen.GWEN_DB_GetNextVar.restype = GWEN_DB_Node
gwen.GWEN_DB_GetGroup.restype = GWEN_DB_Node
gwen.GWEN_DB_GetGroup.argtypes = GWEN_DB_Node, c_int, c_char_p
gwen.GWEN_DB_GetFirstGroup.restype = GWEN_DB_Node
gwen.GWEN_DB_GetNextGroup.restype = GWEN_DB_Node

class GWEN_DB_GroupNode(GWEN_DB_Node):

    def __init__(self):
        raise RuntimeError('please add code to create a group node')

    def deleteVar(self, path):
        res = gwen.GWEN_DB_DeleteVar(self, path)
        if res != 0:
            raise ValueError('DeleteVar failed')

    name = property(gwen.GWEN_DB_GroupName)

gwen.GWEN_DB_DeleteVar.argtypes = GWEN_DB_GroupNode, c_char_p
gwen.GWEN_DB_GroupName.restype = c_char_p


class GWEN_DB_VariableNode(GWEN_DB_Node):

    def __init__(self):
        raise RuntimeError('please add code to create a variable node')

    name = property(gwen.GWEN_DB_VariableName)

gwen.GWEN_DB_VariableName.restype = c_char_p


class GWEN_DB_ValueNode(GWEN_DB_Node):

    def __init__(self):
        assert False, "class GWEN_DB_ValueNode is abstract"

    def __repr__(self):
	return "<%s %s>" % (self.__class__.__name__, self.value)


class GWEN_DB_CharValueNode(GWEN_DB_ValueNode):

    def __init__(self):
        raise RuntimeError('please add code to create a value node')

    value = property(gwen.GWEN_DB_GetCharValueFromNode)

gwen.GWEN_DB_GetCharValueFromNode.restype = c_char_p


class GWEN_DB_IntValueNode(GWEN_DB_ValueNode):

    def __init__(self):
        raise RuntimeError('please add code to create a value node')

    value = property(gwen.GWEN_DB_GetIntValueFromNode)

gwen.GWEN_DB_GetIntValueFromNode.restype = c_int


class GWEN_DB_BinValueNode(GWEN_DB_ValueNode):

    def __init__(self):
        raise RuntimeError('please add code to create a value node')

    def value(self):
        size = c_int()
        v = gwen.GWEN_DB_GetBinValueFromNode(self, byref(size))
        return v[:size]
    value = property(value)

gwen.GWEN_DB_GetBinValueFromNode.restype = c_char_p


class GWEN_DB_PtrValueNode(GWEN_DB_ValueNode):

    def __init__(self):
        raise RuntimeError('please add code to create a value node')

    def value(self):
        return gwen.GWEN_DB_GetPtrValue(self, 0, '', None)
    value = property(value)

gwen.GWEN_DB_GetPtrValue.restype = c_void_p


GWEN_DB_Node._typetable = {
    GWEN_DB_Node.VALUETYPE_CHAR: GWEN_DB_CharValueNode,
    GWEN_DB_Node.VALUETYPE_INT: GWEN_DB_IntValueNode,
    GWEN_DB_Node.VALUETYPE_BIN: GWEN_DB_BinValueNode,
    GWEN_DB_Node.VALUETYPE_PTR: GWEN_DB_PtrValueNode,
    }


################################################################
# Account
#
class Provider(c_void_p):
    """aqBanking Provider (like aqhbci)
    """
    def _check_retval_(p):
        if not p:
            chk(AB_Error.not_available)
        v = Provider.__new__(Provider)
        c_void_p.__init__(v, p)
        return v
    _check_retval_ = staticmethod(_check_retval_)

    def __init__(self):
        raise RuntimeError("class Provider can't be instanciated directly")

    name = property(aqb.AB_Provider_GetName)

Provider.from_param = from_cls_param
aqb.AB_Provider_GetName.restype = c_char_p


class Account(c_void_p):
    """local bank account.

    Instances are created by BaseBanking
    """
    def _check_retval_(p):
        if not p:
            chk(AB_Error.unknown_account)
        v = Account.__new__(Account)
        c_void_p.__init__(v, p)
        return v
    _check_retval_ = staticmethod(_check_retval_)
    
    def __init__(self):
        raise RuntimeError("class Account can't be instanciated directly")

    uniqueId = property(aqb.AB_Account_GetUniqueId)

    accountNumber = property(aqb.AB_Account_GetAccountNumber)

    bankCode = property(aqb.AB_Account_GetBankCode)

    provider = property(aqb.AB_Account_GetProvider)

    accountName = property(aqb.AB_Account_GetAccountName)

    currency = property(aqb.AB_Account_GetCurrency)

    ownerName = property(aqb.AB_Account_GetOwnerName)

    accountType = property(
        aqb.AB_Account_GetAccountType,
        aqb.AB_Account_SetAccountType)

    #def checkAvailability(self):
    #    return aqb.AB_Account_CheckAvailability(self)

    def __eq__(self, other):
        if not isinstance(other, Account):
            return False
        return self.uniqueId == other.uniqueId

    def __str__(self):
	return "Account(%d: %s / %s)" % \
            (self.uniqueId, self.accountNumber, self.bankCode)

Account.from_param = from_cls_param

aqb.AB_Account_GetAccountNumber.restype = c_char_p
aqb.AB_Account_GetBankCode.restype = c_char_p
aqb.AB_Account_GetAccountType.restype = AccountTypeAdapter
aqb.AB_Account_SetAccountType.argtypes = Account, AccountTypeAdapter
aqb.AB_Account_GetProvider.restype = Provider
aqb.AB_Account_GetOwnerName.restype = c_char_p
aqb.AB_Account_GetCurrency.restype = c_char_p
aqb.AB_Account_GetAccountName.restype = c_char_p
#aqb.AB_Account_CheckAvailability.restype = AB_Error


################################################################
# AccountStatus
#
class Balance(c_void_p):
    "wraps AB_BALANCE"

    def _check_retval_(p):
	if not p:
	    return None
        v = Balance.__new__(Balance)
        c_void_p.__init__(v, p)
        return v
    _check_retval_ = staticmethod(_check_retval_)

    def __init__(self, value, time):
	bal = aqb.AB_Balance_new(value, time)
        assert bal
	self.__del__ = aqb.AB_Balance_free
	c_void_p.__init__(self, bal)

    value = property(aqb.AB_Balance_GetValue)
    time = property(aqb.AB_Balance_GetTime)

    def __eq__(self, bal):
	return self.value == bal.value and self.time == bal.time

    def __str__(self):
        return "Balance(%s: %s)" % (self.time, self.value)

aqb.AB_Balance_new.argtypes = [Value, GWEN_Time]
aqb.AB_Balance_GetTime.restype = GWEN_Time
aqb.AB_Balance_GetValue.restype = Value


class AccountStatus(c_void_p):
    """account status return by executing JobGetBalance.

    wraps AB_ACCOUNT_STATUS
    """
    def _check_retval_(p):
        if not p:
            return None
        v = AccountStatus.__new__(AccountStatus)
        c_void_p.__init__(v, p)
        return v
    _check_retval_ = staticmethod(_check_retval_)

    def __init__(self):
        raise RuntimeError(
            "class AccountStatus can't be instanciated directly")

    time = property(aqb.AB_AccountStatus_GetTime)
    bankLine = property(aqb.AB_AccountStatus_GetBankLine)
    disposable = property(aqb.AB_AccountStatus_GetDisposable)
    disposed = property(aqb.AB_AccountStatus_GetDisposed)
    bookedBalance = property(aqb.AB_AccountStatus_GetBookedBalance)
    notedBalance = property(aqb.AB_AccountStatus_GetNotedBalance)

AccountStatus.from_param = from_cls_param

aqb.AB_AccountStatus_GetBookedBalance.restype = Balance
aqb.AB_AccountStatus_GetNotedBalance.restype = Balance
aqb.AB_AccountStatus_GetTime.restype = GWEN_Time
aqb.AB_AccountStatus_GetBankLine.restype = Value
aqb.AB_AccountStatus_GetDisposable.restype = Value
aqb.AB_AccountStatus_GetDisposed.restype = Value


################################################################
# Im- / Export
#
class AccountInfo(c_void_p):
    "Imported data for an account"

    def _check_retval_(p):
        if not p:
            return None
        v = AccountInfo.__new__(AccountInfo)
        c_void_p.__init__(v, p)
        return v
    _check_retval_ = staticmethod(_check_retval_)

    def __init__(self):
        p = aqb.AB_ImExporterAccountInfo_new()
        assert p
        c_void_p.__init__(self, p)
        self.__del__ = aqb.AB_ImExporterAccountInfo_free

    bankCode = property(
        aqb.AB_ImExporterAccountInfo_GetBankCode,
        aqb.AB_ImExporterAccountInfo_SetBankCode)

    bankName = property(
        aqb.AB_ImExporterAccountInfo_GetBankName,
        aqb.AB_ImExporterAccountInfo_SetBankName)

    accountNumber = property(
        aqb.AB_ImExporterAccountInfo_GetAccountNumber,
        aqb.AB_ImExporterAccountInfo_SetAccountNumber)

    accountName = property(
        aqb.AB_ImExporterAccountInfo_GetAccountName,
        aqb.AB_ImExporterAccountInfo_SetAccountName)

    owner = property(
        aqb.AB_ImExporterAccountInfo_GetOwner,
        aqb.AB_ImExporterAccountInfo_SetOwner)

    accountType = property(
        aqb.AB_ImExporterAccountInfo_GetType,
        aqb.AB_ImExporterAccountInfo_SetType)

    description = property(
        aqb.AB_ImExporterAccountInfo_GetDescription,
        aqb.AB_ImExporterAccountInfo_SetDescription)

    def iterTransactions(self):
        "iterates over all the transactions of the account"
	tr = aqb.AB_ImExporterAccountInfo_GetFirstTransaction(self)
	while tr:
	    yield Transaction._check_retval_(tr)
	    tr = aqb.AB_ImExporterAccountInfo_GetNextTransaction(self)

    def iterAccountStatus(self):
        as = aqb.AB_ImExporterAccountInfo_GetFirstAccountStatus(self)
        while as:
            yield AccountStatus._check_retval_(as)
            as = aqb.AB_ImExporterAccountInfo_GetNextAccountStatus(self)

    def addTransaction(self, tr):
        aqb.AB_ImExporterAccountInfo_AddTransaction(self, tr)

    def addAccountStatus(self, as):
        aqb.AB_ImExporterAccountInfo_AddAccountStatus(self, as)

AccountInfo.from_param = from_cls_param

aqb.AB_ImExporterAccountInfo_GetAccountNumber.restype = c_char_p
aqb.AB_ImExporterAccountInfo_GetBankCode.restype = c_char_p
aqb.AB_ImExporterAccountInfo_GetAccountNumber.restype = c_char_p
aqb.AB_ImExporterAccountInfo_GetDescription.restype = c_char_p
aqb.AB_ImExporterAccountInfo_SetDescription.argtypes = AccountInfo, c_char_p
aqb.AB_ImExporterAccountInfo_GetType.restype = AccountTypeAdapter
aqb.AB_ImExporterAccountInfo_SetType.argtypes = AccountInfo, AccountTypeAdapter
aqb.AB_ImExporterAccountInfo_GetOwner.restype = c_char_p
aqb.AB_ImExporterAccountInfo_SetOwner.argtypes = AccountInfo, c_char_p
aqb.AB_ImExporterAccountInfo_GetAccountName.restype = c_char_p
aqb.AB_ImExporterAccountInfo_SetAccountName.argtypes = AccountInfo, c_char_p
aqb.AB_ImExporterAccountInfo_GetBankName.restype = c_char_p
aqb.AB_ImExporterAccountInfo_SetBankName.argtypes = AccountInfo, c_char_p
aqb.AB_ImExporterAccountInfo_AddTransaction.argtypes = AccountInfo, Transaction
aqb.AB_ImExporterAccountInfo_AddAccountStatus.argtypes = (
    AccountInfo, AccountStatus)


class ImExporterContext(c_void_p):
    "container to hold data for im/export"

    def __init__(self):
	ctx = aqb.AB_ImExporterContext_new()
        assert ctx
        c_void_p.__init__(self, ctx)

    def __del__(self):
        aqb.AB_ImExporterContext_free(self)

    def iterAccountInfo(self):
        "iterate over data by accounts"
	ai = aqb.AB_ImExporterContext_GetFirstAccountInfo(self)
	while ai:
	    yield AccountInfo._check_retval_(ai)
	    ai = aqb.AB_ImExporterContext_GetNextAccountInfo(self)

    def addAccountInfo(self, ai):
        aqb.AB_ImExporterContext_AddAccountInfo(self, ai)
        del ai.__del__

aqb.AB_ImExporterContext_AddAccountInfo.argtypes = (
    ImExporterContext, AccountInfo)


class BufferedIO(c_void_p):
    "aqBanking io abstraction"

    def __init__(self, source):
	if hasattr(source, 'fileno') and hasattr(source, 'mode'):
	    c_void_p.__init__(
                self, gwen.GWEN_BufferedIO_File_new(source.fileno()))
            if source.mode[0] == 'r':
                gwen.GWEN_BufferedIO_SetReadBuffer(self, 0, 1024)
            else:
                gwen.GWEN_BufferedIO_SetWriteBuffer(self, 0, 1024)
	else:
	    raise ValueError, "don't know how to handle %s" % type(source)

    def __del__(self):
        gwen.GWEN_BufferedIO_free(self)

BufferedIO.from_param = from_cls_param


class ImExporter(c_void_p):
    "Importer or Exporter for a specific format"

    def _check_retval_(p):
        if not p:
            chk(AB_Error.not_available)
        v = ImExporter.__new__(ImExporter)
        c_void_p.__init__(v, p)
        return v
    _check_retval_ = staticmethod(_check_retval_)

    def __init__(self):
        raise RuntimeError("class Provider can't be instanciated directly")

    name = property(aqb.AB_ImExporter_GetName)

    def checkFile(self, fname):
        return AB_Error(aqb.AB_ImExporter_CheckFile(self, fname))

    def importIntoContext(self, source, dbProfile, ctx = None):
	if ctx is None:
	    ctx = ImExporterContext()
	aqb.AB_ImExporter_Import(self, ctx, BufferedIO(source), dbProfile)
	return ctx

    def exportFromContext(self, dest, dbProfile, ctx):
        chk(aqb.AB_ImExporter_Export(self, ctx, BufferedIO(dest), dbProfile))

ImExporter.from_param = from_cls_param

aqb.AB_ImExporter_CheckFile.argtypes = ImExporter, c_char_p
aqb.AB_ImExporter_GetName.restype = c_char_p


class PluginDescription(c_void_p):
    "describes a plugin (e.g. an ImExporter)"
    def _check_retval_(p):
        if not p:
            chk(AB_Error.not_avaiable)
        v = PluginDescription.__new__(PluginDescription)
        c_void_p.__init__(v, p)
        v.free = gwen.GWEN_PluginDescription_free
        return v
    _check_retval_ = staticmethod(_check_retval_)

    def __init__(self):
        raise RuntimeError(
            "class PluginDescription can't be instanciated directly")

    def __del__(self):
        self.free(self)

    name = property(gwen.GWEN_PluginDescription_GetName)
    type = property(gwen.GWEN_PluginDescription_GetType)
    version = property(gwen.GWEN_PluginDescription_GetVersion)
    author = property(gwen.GWEN_PluginDescription_GetAuthor)
    shortDescr = property(gwen.GWEN_PluginDescription_GetShortDescr)

    def __str__(self):
        return "<%s %s %s / %s %s %s>" % (self.__class__.__name__, self.name,
                                          self.version, self.type,
                                          self.author, self.shortDescr)

PluginDescription.from_param = from_cls_param
gwen.GWEN_PluginDescription_GetName.restype = c_char_p
gwen.GWEN_PluginDescription_GetType.restype = c_char_p
gwen.GWEN_PluginDescription_GetVersion.restype = c_char_p
gwen.GWEN_PluginDescription_GetAuthor.restype = c_char_p
gwen.GWEN_PluginDescription_GetShortDescr.restype = c_char_p


################################################################
# Jobs
################################################################

class JobType(Enum):
    "Job Types"
    (unknown,         # unknown job
     balance,         # retrieve the balance of an online account
     getTransactions, # retrieve transaction statements for an online account
     transfer,        # issue a transfer
     debitNote,       # issue a debit note (Lastschrift)
     euTransfer,      # issue a EU-wide transfer
     ) = range(6)

class JobStatus(Enum):
    "status of job execution"
    (new,
     updated,
     enqueued,
     sent,
     pending,
     finished,
     error,
     deferred,
     ) = range(8)
    unknown = 999

class Job(c_void_p):
    "base class for all banking jobs"

    def _check_retval_(job):
        "creates Job subclass instances from a C pointer - DO NOT USE DIRECTLY"
        assert job
        aqb.AB_Job_Attach(job)
        j = Job.__new__(Job._typetable[aqb.AB_Job_GetType(job)])
        Job.__init__(j,job)
        return j
    _check_retval_ = staticmethod(_check_retval_)

    def __init__(self, job):
        assert self.__class__ != Job, "class Job is abstract"
        c_void_p.__init__(self, job)

    def __del__(self):
        aqb.AB_Job_free(self)

    dele = __del__

    jobid = property(aqb.AB_Job_GetJobId)

    jobtype = property(aqb.AB_Job_GetType)

    jobstatus = property(aqb.AB_Job_GetStatus)

    lastStatusChange = property(aqb.AB_Job_GetLastStatusChange)

    createdBy = property(aqb.AB_Job_GetCreatedBy)

    account = property(aqb.AB_Job_GetAccount)

    def checkAvailability(self):
        return aqb.AB_Job_CheckAvailability(self)

    def __str__(self):
        return "%s(id=%d/%s, type=%s, status=%s(%s), mem=0x%x)" % \
               (self.__class__.__name__, self.jobid, self.createdBy,
                self.jobtype, self.jobstatus, self.lastStatusChange, self.value)

def from_param(cls, value):
    if not isinstance(value, Job):
        raise TypeError('class Job expected (found %s)' % value)
    return value
Job.from_param = classmethod(from_param)

aqb.AB_Job_GetLastStatusChange.restype = GWEN_Time
aqb.AB_Job_GetAccount.restype = Account
aqb.AB_Job_GetCreatedBy.restype = c_char_p
aqb.AB_Job_GetStatus.restype = JobStatus
aqb.AB_Job_CheckAvailability.restype = AB_Error
aqb.AB_Job_GetType.restype = JobType


class JobSingleDebitNote(Job):
    "retrieve money from another account"

    def __init__(self, account, transaction):
        Job.__init__(self, aqb.AB_JobSingleDebitNote_new(account))
        self.transaction = transaction

    transaction = property(
        aqb.AB_JobSingleDebitNote_GetTransaction,
        aqb.AB_JobSingleDebitNote_SetTransaction)

    fieldLimits = property(aqb.AB_JobSingleDebitNote_GetFieldLimits)

    #def iterTextKeys(self):
    #    p = aqb.AB_JobSingleDebitNote_GetTextKeys(self)
    #    if not p:
    #        chk(AB_Error.not_available)
    #    for i in p:
    #        if i < 0:
    #            break
    #        yield i

    #maxPurposeLines = property(aqb.AB_JobSingleDebitNote_GetMaxPurposeLines)

aqb.AB_JobSingleDebitNote_GetTransaction.restype = Transaction
aqb.AB_JobSingleDebitNote_GetFieldLimits.restype = TransactionLimits
#aqb.AB_JobSingleDebitNote_GetTextKeys.restype = c_int_p
#aqb.AB_JobSingleDebitNote_GetMaxPurposeLines.restype = c_int


class JobSingleTransfer(Job):
    "transfer money from your account to another"

    def __init__(self, account, transaction):
        Job.__init__(self, aqb.AB_JobSingleTransfer_new(account))
        self.transaction = transaction

    transaction = property(
        aqb.AB_JobSingleTransfer_GetTransaction,
        aqb.AB_JobSingleTransfer_SetTransaction)

    fieldLimits = property(aqb.AB_JobSingleTransfer_GetFieldLimits)

    #def iterTextKeys(self):
    #    p = aqb.AB_JobSingleTransfer_GetTextKeys(self)
    #    if not p:
    #        chk(AB_Error.not_available)
    #    for i in p:
    #        if i < 0:
    #            break
    #        yield i

    #maxPurposeLines = property(aqb.AB_JobSingleTransfer_GetMaxPurposeLines)

aqb.AB_JobSingleTransfer_GetTransaction.restype = Transaction
aqb.AB_JobSingleTransfer_GetFieldLimits.restype = TransactionLimits
#aqb.AB_JobSingleTransfer_GetTextKeys.restype = c_int_p
#aqb.AB_JobSingleTransfer_GetMaxPurposeLines.restype = c_int


class JobEuTransfer(Job):
    "transfer money from your account to another, EU-wide"

    class ChargeWhom(Enum):
        (unknown,
         local,
         remote,
         share,
         ) = range(4)

    def __init__(self, account, transaction):
        Job.__init__(self, aqb.AB_JobEuTransfer_new(account))
        self.transaction = transaction

    transaction = property(
        aqb.AB_JobEuTransfer_GetTransaction,
        aqb.AB_JobEuTransfer_SetTransaction)

    chargeWhom = property(
        aqb.AB_JobEuTransfer_GetChargeWhom,
        aqb.AB_JobEuTransfer_SetChargeWhom)

    ibanAllowed = property(aqb.AB_JobEuTransfer_GetIbanAllowed)

    def findCountryInfo(self, cnt):
        return aqb.AB_JobEuTransfer_FindCountryInfo(self, cnt)

    def iterCountryInfo(self):
	cl = aqb.AB_JobEuTransfer_GetCountryInfoList(self)
	if not sl:
	    return
	e = aqb.AB_EuTransferInfo_List_First(sl)
	while e:
	    yield EuTransferInfo._check_retval_(e)
	    e = aqb.AB_EuTransferInfo_List_Next(e)


class ChargeWhomAdapter(c_int):
    def _check_retval_(i):
        return JobEuTransfer.ChargeWhom(i)

def from_param(cls, e):
    check_enum(e, JobEuTransfer.ChargeWhom, 'argument')
    return int(e)
ChargeWhomAdapter.from_param = classmethod(from_param)

aqb.AB_JobEuTransfer_GetTransaction.restype = Transaction
aqb.AB_JobEuTransfer_GetChargeWhom.restype = ChargeWhomAdapter
aqb.AB_JobEuTransfer_SetChargeWhom.argtypes = (
    JobEuTransfer, ChargeWhomAdapter)
aqb.AB_JobEuTransfer_FindCountryInfo.restype = EuTransferInfo
aqb.AB_JobEuTransfer_FindCountryInfo.argtypes = JobEuTransfer, c_char_p


class JobGetTransactions(Job):
    "get the transactions for an account"

    def __init__(self, account):
        Job.__init__(self, aqb.AB_JobGetTransactions_new(account))

    fromTime = property(
        aqb.AB_JobGetTransactions_GetFromTime,
        aqb.AB_JobGetTransactions_SetFromTime)

    toTime = property(
        aqb.AB_JobGetTransactions_GetToTime,
        aqb.AB_JobGetTransactions_SetToTime)

    def iterTransactions(self):
        tl = aqb.AB_JobGetTransactions_GetTransactions(self)
        if not tl:
            return
        tit = aqb.AB_Transaction_List2_First(tl)
        tp = aqb.AB_Transaction_List2Iterator_Data(tit)
        while tp:
	    tr = Transaction._check_retval_(tp)
            tr._owner = self
            yield tr
            tp = aqb.AB_Transaction_List2Iterator_Next(tit)
        aqb.AB_Transaction_List2Iterator_free(tit)

aqb.AB_JobGetTransactions_GetFromTime.restype = GWEN_Time
aqb.AB_JobGetTransactions_SetFromTime.argtypes = [Job, GWEN_Time]
aqb.AB_JobGetTransactions_GetToTime.restype = GWEN_Time
aqb.AB_JobGetTransactions_SetToTime.argtypes = [Job, GWEN_Time]

class JobGetBalance(Job):
    "get balance for an account"

    def __init__(self, account):
        Job.__init__(self, aqb.AB_JobGetBalance_new(account))

    accountStatus = property_addref(
        aqb.AB_JobGetBalance_GetAccountStatus)


aqb.AB_JobGetBalance_GetAccountStatus.restype = AccountStatus

Job._typetable = {
    JobType.balance: JobGetBalance,
    JobType.getTransactions: JobGetTransactions,
    JobType.transfer: JobSingleTransfer,
    JobType.debitNote: JobSingleDebitNote,
    JobType.euTransfer: JobEuTransfer,
    }


################################################################
# User Interface
#

class LogLevel(Enum):
    "level of messages for display to the user"
    (panic,
     emergency,
     error,
     warn,
     notice,
     info,
     debug,
     verbous,
     ) = range(8)

class PinStatus(Enum):
    (bad,
     unknown,
     ok,
     ) = range(-1,2)

class TanStatus(Enum):
    (bad,
     unknown,
     used,
     unused,
     ) = range(-1,3)

def stripText(text, markup=True, encoding=None):
    "return the text or html part of a message"
    i = text.find('<html>')
    if markup:
        if i >= 0:
            text = text[i+6:]
            i = text.rfind('</html>')
            if i > 0:
                text = text[:i]
            text = re.sub(' *</?p> *', '',text)
            text = re.sub(' *\n *', '\n', text)
    elif i > 0:
        text = text[:i]
    text = text.decode('utf8')
    if encoding:
        text = text.encode(encoding)
    return text

MSG_FLAGS_TYPE_MASK  = 0x07
MSG_FLAGS_TYPE_INFO  = 0
MSG_FLAGS_TYPE_WARN  = 1
MSG_FLAGS_TYPE_ERROR = 2

MSG_FLAGS_CONFIRM_B1 = 1 << 3
MSG_FLAGS_CONFIRM_B2 = 2 << 3

MSG_FLAGS_SEVERITY_MASK      = 0x7 << 5
MSG_FLAGS_SEVERITY_NORMAL    = 0x0 << 5
MSG_FLAGS_SEVERITY_DANGEROUS = 0x1 << 5

def msg_flags_type(fl):
    return fl & MSG_FLAGS_TYPE_MASK

def msg_flags_confirm_button(fl):
    return (fl & 0x3) >> 3

################################################################
# Base class for main Banking Object
#
class BankingBase(c_void_p):
    """abstract base for the main banking class.

    cf. console.BankingConsole and gtkui.BankingGtk for useable classes.
    """
    callback_funcs = (

        ('getPin', aqb.AB_Banking_SetGetPinFn, CFUNCTYPE(
    	c_int, c_void_p, c_int, c_char_p, c_char_p,
        c_char_p, c_void_p, c_int, c_int)),

        ('messageBox', aqb.AB_Banking_SetMessageBoxFn, CFUNCTYPE(
        c_int, c_void_p, c_int, c_char_p, c_char_p,
        c_char_p, c_char_p, c_char_p)),

        ('inputBox', aqb.AB_Banking_SetInputBoxFn, CFUNCTYPE(
        c_int, c_void_p, c_int, c_char_p, c_char_p, c_char_p, c_int, c_int)),

        ('showBox', aqb.AB_Banking_SetShowBoxFn, CFUNCTYPE(
        c_int, c_void_p, c_int, c_char_p, c_char_p)),

        ('hideBox', aqb.AB_Banking_SetHideBoxFn, CFUNCTYPE(
        None, c_void_p, c_int)),

        ('progressStart', aqb.AB_Banking_SetProgressStartFn, CFUNCTYPE(
        c_int, c_void_p, c_char_p, c_char_p, c_int)),

        ('progressAdvance', aqb.AB_Banking_SetProgressAdvanceFn, CFUNCTYPE(
        c_int, c_void_p, c_int, c_int)),

        ('progressLog', aqb.AB_Banking_SetProgressLogFn, CFUNCTYPE(
        c_int, c_void_p, c_int, c_int, c_char_p)),

        ('progressEnd', aqb.AB_Banking_SetProgressEndFn, CFUNCTYPE(
        c_int, c_void_p, c_int)),

        ('printout', aqb.AB_Banking_SetPrintFn, CFUNCTYPE(
        c_int, c_void_p, c_char_p, c_char_p, c_char_p, c_char_p, c_char_p)),

        ('setPinStatus', aqb.AB_Banking_SetSetPinStatusFn, CFUNCTYPE(
        c_int, c_void_p, c_char_p, c_char_p, c_int)),

        ('getTan', aqb.AB_Banking_SetGetTanFn, CFUNCTYPE(
        c_int, c_void_p, c_char_p, c_char_p, c_char_p, c_char_p,
        c_int, c_int)),

        ('setTanStatus', aqb.AB_Banking_SetSetTanStatusFn, CFUNCTYPE(
        c_int, c_void_p, c_char_p, c_char_p, c_int)),
        )

    def __init__(self, name, configdir=None):
        global aqb
	c_void_p.__init__(self, aqb.AB_Banking_new(name, configdir))
        self.inited = False
        self.bankingFree = aqb.AB_Banking_free
        l = []
        for name, fn, tp in BankingBase.callback_funcs:
            try:
                f = getattr(self, name)
            except AttributeError:
                if self.__class__ == BankingBase:
                    raise RuntimeError('class BankingBase is abstract!')
                raise AttributeError(
                    'callback %s must be defined in class %s' % (name, self.__class__))
            f = tp(f)
            fn(self, f)
            l.append(f)
        self.func_refs = l
        # never unregistered (FIXME? could use 1 func + list + callback)
        atexit.register(lambda r=weakref.ref(self): r() and r().fini())
	chk(aqb.AB_Banking_Init(self))
        self.inited = True

    def fini(self):
        if self.inited:
            self.inited = False
            chk(aqb.AB_Banking_Fini(self))

    def save(self):
        chk(aqb.AB_Banking_Save(self))

    def __del__(self):
        self.fini()
        self.bankingFree(self)

    appName = property(aqb.AB_Banking_GetAppName)

    def getProviderDescrs(self):
        pl = aqb.AB_Banking_GetProviderDescrs(self)
        if not pl:
            return
        pit = gwen.GWEN_PluginDescription_List2_First(pl)
        pd = gwen.GWEN_PluginDescription_List2Iterator_Data(pit)
        while pd:
            yield PluginDescription._check_retval_(pd)
            pd = gwen.GWEN_PluginDescription_List2Iterator_Next(pit)
        gwen.GWEN_PluginDescription_List2Iterator_free(pit)
        gwen.GWEN_PluginDescription_List2_free(pl)

    def iterActiveProviders(self):
        return aqb.AB_Banking_GetActiveProviders(self)

    def activateProvider(self, backend):
        chk(aqb.AB_Banking_ActivateProvider(self, backend))

    def deactivateProvider(self, backend):
        chk(aqb.AB_Banking_DeactivateProvider(self, backend))

    def getAppUserDataDir(self):
        buf = GWEN_Buffer()
        chk(aqb.AB_Banking_GetAppUserDataDir(self, buf))
        return str(buf)

    appUserDataDir = property(getAppUserDataDir)

    appData = property(aqb.AB_Banking_GetAppData)

    def findWizard(self, backend, frontends='gnome;gtk;qt;kde'):
        buf = GWEN_Buffer()
        chk(aqb.AB_Banking_FindWizard(self, backend, frontends, buf))
        return str(buf)

    def iterImExporterDescrs(self):
	pl = aqb.AB_Banking_GetImExporterDescrs(self)
        if not pl:
            return
	pit = gwen.GWEN_PluginDescription_List2_First(pl)
	pd = gwen.GWEN_PluginDescription_List2Iterator_Data(pit)
	while pd:
	    yield PluginDescription._check_retval_(pd)
	    pd = gwen.GWEN_PluginDescription_List2Iterator_Next(pit)
        gwen.GWEN_PluginDescription_List2Iterator_free(pit)
        gwen.GWEN_PluginDescription_List2_free(pl)
        
    def getImExporter(self, tp):
	return aqb.AB_Banking_GetImExporter(self, tp)

    def getImExporterProfiles(self, tp):
	return aqb.AB_Banking_GetImExporterProfiles(self, tp)

    def getAccountByCodeAndNumber(self, bank, account):
	return aqb.AB_Banking_GetAccountByCodeAndNumber(
            self, bank, account)

    def enqueueJob(self, job):
    	chk(aqb.AB_Banking_EnqueueJob(self, job))

    def deferJob(self, job):
        chk(aqb.AB_Banking_DeferJob(self, job))

    def enqueuePendingJobs(self, mineOnly):
        chk(aqb.AB_Banking_EnqueuePendingJobs(self, mineOnly))

    def iterEnqueuedJobs(self):
        return aqb.AB_Banking_GetEnqueuedJobs(self)

    def iterFinishedJobs(self):
        return aqb.AB_Banking_GetFinishedJobs(self)

    def iterDeferredJobs(self):
        return aqb.AB_Banking_GetDeferredJobs(self)

    def iterPendingJobs(self):
        return aqb.AB_Banking_GetPendingJobs(self)

    def iterArchivedJobs(self):
        return aqb.AB_Banking_GetArchivedJobs(self)

    def delArchivedJob(self, job):
        chk(aqb.AB_Banking_DelArchivedJob(self, job))

    def delPendingJob(self, job):
        chk(aqb.AB_Banking_DelPendingJob(self, job))

    def delFinishedJob(self, job):
        chk(aqb.AB_Banking_DelFinishedJob(self, job))

    def delDeferredJob(self, job):
        chk(aqb.AB_Banking_DelDeferredJob(self, job))

    def dequeueJob(self, job):
	chk(aqb.AB_Banking_DequeueJob(self, job))

    def executeQueue(self):
	chk(aqb.AB_Banking_ExecuteQueue(self))

    def requestBalance(self, bankCode, accountNumber):
	chk(aqb.AB_Banking_RequestBalance(self, bankCode, accountNumber))

    def requestTransactions(self, bankCode, accountNumber, firstDate, lastDate):
        chk(aqb.AB_Banking_RequestTransactions(
            self, bankCode, accountNumber, firstDate, lastDate))

    def gatherResponses(self, context):
        chk(aqb.AB_Banking_GatherResponses(self, context))

    def iterAccounts(self):
	al = aqb.AB_Banking_GetAccounts(self)
        if not al:
            return
        ait = aqb.AB_Account_List2_First(al)
        ap = aqb.AB_Account_List2Iterator_Data(ait)
 	while ap:
 	    yield Account._check_retval_(ap)
 	    ap = aqb.AB_Account_List2Iterator_Next(ait)
        gwen.GWEN_PluginDescription_List2Iterator_free(ait)
        gwen.GWEN_PluginDescription_List2_free(al)

    def getAccount(self, uniqueId):
	return aqb.AB_Banking_GetAccount(self, uniqueId)

    def getAlias(self, account, subst_id=True):
        """get the alias name of an account.

        account    Account instance or uniqueId
        subst_id   if no alias is found: if True return
                   the uniqueId as string, else return None
        """
        if isinstance(account, Account):
            aid = account.uniqueId
        for i, a in self.iterAliases():
            if i == aid:
                return a
        if subst_id:
            return str(aid)
        return None

    def setAccountAlias(self, account, alias):
        """Set an alias name for an account

        deletes all previous aliases for this account
        """
        n = self._getAliasNode()
        if n is not None:
            aid = account.uniqueId
            aliases = []
            for v in n.iterVars():
                if v.getIntValue() == aid:
                    aliases.append(v.name)
            for v in aliases:
                n.deleteVar(v)
        aqb.AB_Banking_SetAccountAlias(self, account, alias)

    def getAccountByAlias(self, alias):
        return aqb.AB_Banking_GetAccountByAlias(self, alias)

    def _getAliasNode(self):
        return self.appData.getGroup(PathFlags.NAMEMUSTEXIST,'banking/aliases')

    def iterAliases(self):
        n = self._getAliasNode()
        if n is None:
            return
        for v in n.iterVars():
            yield v.getIntValue(), v.name


BankingBase.from_param = from_cls_param

aqb.AB_Banking_GetActiveProviders.restype = iterStringlist
aqb.AB_Banking_GetEnqueuedJobs.restype = iterJobs
aqb.AB_Banking_GetPendingJobs.restype = iterJobs
aqb.AB_Banking_GetFinishedJobs.restype = iterJobs
aqb.AB_Banking_GetArchivedJobs.restype = iterJobs
aqb.AB_Banking_GetAppName.restype = c_char_p
aqb.AB_Banking_GetAccountByCodeAndNumber.restype = Account
aqb.AB_Banking_GetAccount.restype = Account
aqb.AB_Banking_GetAppData.restype = GWEN_DB_Node
aqb.AB_Banking_ActivateProvider.argtypes = BankingBase, c_char_p
aqb.AB_Banking_DeactivateProvider.argtypes = BankingBase, c_char_p
aqb.AB_Banking_GetAccountByAlias.restype = Account
aqb.AB_Banking_GetAccountByAlias.argtypes = BankingBase, c_char_p
aqb.AB_Banking_SetAccountAlias.argtypes = BankingBase, Account, c_char_p
aqb.AB_Banking_GetAccountByCodeAndNumber.argtypes = (
    BankingBase, c_char_p, c_char_p)
aqb.AB_Banking_GetImExporter.restype = ImExporter
aqb.AB_Banking_GetImExporter.argtypes = BankingBase, c_char_p
aqb.AB_Banking_GetImExporterProfiles.restype = GWEN_DB_Node
aqb.AB_Banking_GetImExporterProfiles.argtypes = BankingBase, c_char_p
aqb.AB_Banking_FindWizard.argtypes = (
    BankingBase, c_char_p, c_char_p, GWEN_Buffer)
aqb.AB_Banking_RequestTransactions.argtypes = (
    BankingBase, c_char_p, c_char_p, GWEN_Time, GWEN_Time)
aqb.AB_Banking_GatherResponses.argtypes = BankingBase, ImExporterContext

# FIXME: should not be needed for stable version of aqBanking
# shut up aqBanking.. too noisy..
Logger_SetLevel('aqbanking', LoggerLevel.error)
Logger_SetLevel('gwenhywfar', LoggerLevel.error)
Logger_SetLevel('aqhbci', LoggerLevel.error)
Logger_SetLevel(None, LoggerLevel.error)
