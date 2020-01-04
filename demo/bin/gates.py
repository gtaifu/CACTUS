class _GateNotInput:
    """docstring for _OneGateNotInput"""

    def __init__(self, s=' ', cale=False):
        self._s = s
        self._callable = cale

    def dot(self, other):
        r = type(self)()
        r._s = self._s + '.dot(%s)' % str(other)
        return r

    def __call__(self, theta):
        if self._callable == False:
            raise ValueError('is not callable.')
        if not isinstance(theta, (int, float, complex)):
            raise TypeError('wrong type.')
        r = type(self)()
        r._s = self._s + '(' + str(theta) + ')'
        return r

# guochu add attribute .T, .H .shift and .conj
    @property
    def T(self):
        r = type(self)()
        r._s = self._s + '.T'
        return r

    @property
    def H(self):
        r = type(self)()
        r._s = self._s + '.T.conj()'
        return r

    def conj(self):
        r = type(self)()
        r._s = self._s + '.conj()'
        return r

    def __str__(self):
        return self._s


class _CONTROL(_GateNotInput):
    """docstring for _CONTROL"""

    def __init__(self, s=' ', cale=False):
        super().__init__(s, cale)

    def __call__(self, other):
        if self._callable == False:
            raise ValueError('is not callable.')
        if not isinstance(other, _GateNotInput):
            raise TypeError('wrong type')
        r = type(self)()
        r._s = self._s + '(' + str(other) + ')'
        return r


X = _GateNotInput('X')
Y = _GateNotInput('Y')
Z = _GateNotInput('Z')
H = _GateNotInput('H')
S = _GateNotInput('S')
R = _GateNotInput('R', cale=True)
ROTATION = _GateNotInput('ROTATION', cale=True)
Rx = _GateNotInput('Rx', cale=True)
Ry = _GateNotInput('Ry', cale=True)
Rz = _GateNotInput('Rz', cale=True)
Xh = _GateNotInput('Xh')
Yh = _GateNotInput('Yh')
T = _GateNotInput('T')
CZ = _GateNotInput('CZ')
CNOT = _GateNotInput('CNOT')
SWAP = _GateNotInput('SWAP')
CONTROL = _CONTROL('CONTROL', cale=True)
CONTROLCONTROL = _CONTROL('CONTROLCONTROL', cale=True)
TOFFOLI = _GateNotInput('TOFFOLI')