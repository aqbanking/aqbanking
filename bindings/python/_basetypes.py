import os, datetime, time, sys
from ctypes import c_char_p, c_int, c_uint, c_double, c_void_p, c_double, \
     cdll, CFUNCTYPE, POINTER

if sys.modules['ctypes'].__version__ < '0.9.5':
    print 'Error: Please install at least ctypes V0.9.5 (currently: %s)' \
          % sys.modules['ctypes'].__version__
    raise SystemExit, 1


if os.name == 'nt':
    aqb = cdll.aqbanking32_16
    gwen = cdll.gwenhywfar32_38
elif os.name == 'mac':
    aqb = cdll.aqbanking
    gwen = cdll.gwenhywfar
else:
    aqb = cdll['libaqbanking.so.16']
    gwen = cdll['libgwenhywfar.so.38']


_c_char_p = c_char_p
class c_utf8_p(_c_char_p):
    def _check_retval_(p):
        if p is None:
            return None
        return unicode(_c_char_p(p).value, 'utf8')
    _check_retval_ = staticmethod(_check_retval_)

def from_param(cls, value):
    if value is None:
        return None
    if isinstance(value, unicode):
        return value.encode('utf8')
    elif isinstance(value, str):
        return value
    else:
        raise ValueError("string expected (received: %s)" % type(value))
c_utf8_p.from_param = classmethod(from_param)
c_char_p = c_utf8_p


def from_cls_param(cls, value):
    if not isinstance(value, cls):
        raise TypeError('class %s expected (found %s)'
                        % (cls.__name__, value))
    return value
from_cls_param = classmethod(from_cls_param)


def property_addref(get, set=None, doc=None):
    """like property() but add reference to object.

    The getter adds the field _owner to the result and sets it to
    the object. This is used when the result encapsulates a C pointer
    which becomes invalid when the object is finalised. The extra
    reference guaranties that the object will live at least as long
    as the returned property
    """
    def get_addref(self):
        res = get(self)
        if res is not None:
            res._owner = self
        return res
    return property(get_addref, set, doc)


class Value(c_void_p):
    """A money value.

    Properties value, currency
    """
    def _check_retval_(p):
        if not p:
            return None
        v = Value.__new__(Value)
        c_void_p.__init__(v, p)
        return v
    _check_retval_ = staticmethod(_check_retval_)

    def __init__(self, value, currency):
        v = aqb.AB_Value_new(value, currency)
        assert v
        self.__del__ = aqb.AB_Value_free
        c_void_p.__init__(self, v)

    value = property(
        aqb.AB_Value_GetValue,
        aqb.AB_Value_SetValue)

    currency = property(
        aqb.AB_Value_GetCurrency,
        aqb.AB_Value_SetCurrency)

    def __eq__(self, val):
	return abs(self.value - val.value) < 0.001 \
            and self.currency == val.currency

    def __str__(self):
        return "%.2f %s" % (self.value, self.currency)

Value.from_param = from_cls_param

aqb.AB_Value_new.argtypes = [c_double, c_char_p]
aqb.AB_Value_GetValue.restype = c_double
aqb.AB_Value_SetValue.argtypes = [Value, c_double]
aqb.AB_Value_GetCurrency.restype = c_char_p
aqb.AB_Value_SetCurrency.argtypes = [Value, c_char_p]


class GWEN_Time(c_void_p):
    """Convert GWEN_Time to datetime.

    To be used in .restype and .argtypes.

    For argtypes expects a datetime object and creates a GWEN_TIME
    struct from it which lives as long as the function call. It will
    stay the owner of the GWEN_TIME struct.

    When used as restype it converts the GWEN_TIME struct to datetime
    but does not take ownership of the struct.
    """
    def _check_retval_(gt):
        "used on the return value"
        if not gt:
            return None
        return datetime.datetime.fromtimestamp(gwen.GWEN_Time_Seconds(gt))
    _check_retval_ = staticmethod(_check_retval_)

    def __init__(self):
        raise RuntimeError(
            "class GWEN_Time can't be instanciated directly")

    def __del__(self):
        gwen.GWEN_Time_free(self)

def from_param(cls, value):
    "used on a argument value"
    if not isinstance(value, datetime.datetime):
        raise TypeError('datetime expected (found %s)' % value)
    v = GWEN_Time.__new__(GWEN_Time)
    c_void_p.__init__(v, gwen.GWEN_Time_fromSeconds(
        int(time.mktime(value.timetuple()))))
    return v
GWEN_Time.from_param = classmethod(from_param)
