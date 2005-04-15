# -*- encoding: latin1 -*-
# based on examples/Demo/newmetaclasses/Enum.py of the
# python distribution V2.3.
#
# Copyright for changes:
# Copyright (C) 2005 by Andreas Degert
# see LICENSE.txt in the top level distribution directory
#
"""Enumeration metaclass."""

class EnumMetaclass(type):
    "Metaclass for enumeration."

    def __init__(cls, name, bases, dict):
        cls._members = []
        cls._memberdict = {}
        for attr, value in dict.items():
            if not (attr.startswith('__') and attr.endswith('__')):
                EnumInstance(cls, attr, value)

    def __call__(cls, value):
        try:
            return cls._memberdict[value]
        except KeyError:
            raise ValueError("%s not found in enum %s" % (value, cls.__name__))

    def __repr__(cls):
        if cls._members:
            l = [(getattr(cls, val), val) for val in cls._members]
            l.sort()
            s = ': {%s}' % ', '.join(["%s: %d" % (val, attr)
                                      for attr, val in l])
        else:
            s = ''
        return "%s%s" % (cls.__name__, s)

class EnumInstance(int):
    "Class to represent an enumeration value."

    def __new__(cls, classobj, enumname, value):
        return int.__new__(cls, value)

    def __init__(self, classobj, enumname, value):
        try:
            n = classobj._memberdict[value]
            raise ValueError(
                'Value %d found twice in Enum %s: %s, %s'
                % (value, classobj.__name__, n.__enumname, enumname))
        except KeyError:
            pass
        classobj._memberdict[value] = self
        classobj._members.append(enumname)
        setattr(classobj, enumname, self)
        self.enumclass = classobj
        self.__enumname = enumname

    def __repr__(self):
        return "EnumInstance(%s, %s, %d)" % (
            self.enumclass.__name__, self.__enumname, self)

    def __str__(self):
        return "%s.%s" % (self.enumclass.__name__, self.__enumname)

class Enum:
    """Class to use as a base class for an enumeration.

    class TestEnum(Enum):
        a = 1
        b = 2
    """
    __metaclass__ = EnumMetaclass


def check_enum(value, cls, name):
    if not isinstance(value, EnumInstance) or value.enumclass is not cls:
        raise TypeError("%s must be a %s" % (name, cls.__name__))

if __name__ == '__main__':
    class T(Enum):
        a, b = range(1,3)

    print T
    print T.a
    print T(T.a+1)
    try:
        print T(3)
    except ValueError, e:
        print e
    try:
        class B(Enum):
            a = b = 2
        print B
    except ValueError, e:
        print e
