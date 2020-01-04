# coding=utf-8
import requests
import json
import os
import sys


class _Gate(object):
    """
    serialised output format: "KEY OP"
    """

    def __init__(self, key, op):
        self.key = key
        self.op = op

    def __str__(self):
        return 'Gate(%s, %s)' % (self.key, self.op)

    @property
    def T(self):
        return type(self)(self.key, self.op.T)

    @property
    def H(self):
        return type(self)(self.key, self.op.H)

    def conj(self):
        return type(self)(self.key, self.op.conj())

    def shift(self, distance):
        if not isinstance(distance, int):
            raise TypeError('distance must be an integer.')
        if isinstance(self.key, int):
            return type(self)(self.key + distance, self.op)
        else:
            return type(self)(tuple([s + distance for s in self.key]), self.op)

    def __repr__(self):
        return str(self)


class _Observer(object):
    """
    serialised output format: "KEY OP AUTO_RESET KEEP NAME"
    """

    def __init__(self, key, auto_reset=0, keep=1):
        self.key = key
        self.auto_reset = auto_reset
        self.keep = keep

    def __str__(self):
        return 'Observer(%s, %s, %s)' % (
            self.key, self.auto_reset, self.keep
        )

    def shift(self, distance):
        if not isinstance(distance, int):
            raise TypeError('distance must be an integer.')
        return type(self)(self.key + distance, self.auto_reset, self.keep)

    def __repr__(self):
        return str(self)


class _ClassicalObserver(object):
    """docstring for _ClassicalObserver"""

    def __init__(self, sites, opname, name=None):
        self.sites = sites
        self.opname = opname
        self.name = name

    def shift(self, distance):
        if not isinstance(distance, int):
            raise TypeError('distance must be an integer.')
        return type(self)(tuple([s + distance for s in self.sites]), self.opname, self.name)

    def __str__(self):
        if self.name is None:
            return "ClassicalObserver(%s, %s)" % (
                self.sites, self.opname.encode('utf-8')
            )
        else:
            return "ClassicalObserver(%s, %s, %s)" % (
                self.sites, self.opname.encode('utf-8'), self.name
            )

    def __repr__(self):
        if self.name is None:
            return 'ClassicalObserver(%s, %s)' % (
                self.sites, self.opname
            )
        else:
            return 'ClassicalObserver(%s, %s, %s)' % (
                self.sites, self.opname, self.name
            )


class _Observer2D(object):
    """
    serialised output format: "KEY OP AUTO_RESET KEEP NAME"
    """

    def __init__(self, key):
        self.key = key
        self.op = ''

    def __str__(self):
        # return 'Observer(%s%s)' % (self.key, self.op)
        return 'Observer(%s)' % (self.key,)

    def __repr__(self):
        return str(self)


class _ClassicalObserver2D(object):
    """docstring for _ClassicalObserver"""

    def __init__(self, sites, opname, name=None):
        self.sites = sites
        self.opname = opname
        self.name = name

    def __str__(self):
        if self.name is None:
            return "ClassicalObserver(%s, %s)" % (
                self.sites, self.opname.encode('utf-8')
            )
        else:
            return "ClassicalObserver(%s, %s, %s)" % (
                self.sites, self.opname.encode('utf-8'), self.name
            )

    def __repr__(self):
        if self.name is None:
            return 'ClassicalObserver(%s, %s)' % (
                self.sites, self.opname
            )
        else:
            return 'ClassicalObserver(%s, %s, %s)' % (
                self.sites, self.opname, self.name
            )


class _NotKeyError(Exception):
    def __init__(self, ErrorInfo):
        super().__init__(self)
        self.errorinfo = ErrorInfo

    def __str__(self):
        return self.errorinfo


class _NotAuthError(Exception):
    def __init__(self, ErrorInfo):
        super().__init__(self)
        self.errorinfo = ErrorInfo

    def __str__(self):
        return self.errorinfo


class _RequestError(Exception):
    def __init__(self, ErrorInfo):
        super().__init__(self)
        self.errorinfo = ErrorInfo

    def __str__(self):
        return self.errorinfo


class QState(object):
    def __init__(self, L):
        self.qstates = [0] * L

    def __setitem__(self, key, value):
        if not isinstance(value, (int, tuple, list)):
            raise ValueError('Wrong parameter')
        self.qstates[key] = value

    @property
    def size(self):
        return len(self.qstates)

    def serialize(self):
        return self.qstates


class Auth(object):
    """
    User information
    """

    def __init__(self, ip='120.79.17.42', token=None, port=80):
        self.ip = ip
        self.token = token
        self.port = port


class _Network(object):
    def __init__(self):
        self.nodes = []

    def __len__(self):
        return self.nodes.__len__()

    def __getitem__(self, key):
        if isinstance(key, int):
            return self.nodes[key]
        else:
            r = type(self)()
            r.nodes = self.nodes[key]
            return r

    def __setitem__(self, key, value):
        self.nodes[key] = value

    def __iter__(self):
        return iter(self.nodes)

    def _request(self):
        pass

    def add(self, key, op):
        if key is None and op is None:
            raise _NotKeyError('Invalid parameter')
        gate = _Gate(key, op)
        self.nodes.append(gate)
        return

    def add_observer(self, key=None, auto_reset=1, keep=1):

        raise SystemError('Invalid Method')

    def add_network(self, network):
        if isinstance(network, type(self)):
            self.nodes.extend(network.nodes)
        else:
            self.nodes.extend(network)
        return

    def serialize(self):
        return [str(s) for s in self]

    def _deserialize_result1d(self, result_data):
        # some deserialisation action decealised_result1d
        print("deserialize 1here")
        return result_data.get('message')

    @property
    def T(self):
        r = type(self)()
        r.add_network([s.T for s in self[::-1]])
        return r

    @property
    def H(self):
        r = type(self)()
        r.add_network([s.H for s in self[::-1]])
        return r

    def conj(self):
        r = type(self)()
        r.add_network([s.conj() for s in self])
        return r

    def __str__(self):
        return str(self.nodes)

    def __repr__(self):
        return repr(self.nodes)


class QuantumCircuit(_Network):
    def __init__(self, L, auth, interactive):
        self.L = L
        self.auth = auth
        self.interactive = interactive
        self.states = QState(L=self.L)

        super(QuantumCircuit, self).__init__()

    def __enter__(self):
        self.states = QState(L=self.L)
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        session = requests.session()
        headers = {
            'Accept': 'application/json',
            'Content-Type': 'application/json',
            'Authorization': '%s' % self.auth.token,
        }
        result = session.post(
            url='https://%s/api/v1/circuit/result/' % self.auth.ip,
            data=json.dumps({"end": True}),
            headers=headers
        )
        status = result.status_code
        if status == 200:
            print('[+]感谢使用，祝您生活愉快！')
        else:
            print("[-]请勿重复结束任务")
        return result

    def shift(self, distance):
        r = type(self)()
        r.add_network([s.shift(distance) for s in self])
        return r

    def add_observer(self, key=None, auto_reset=1, keep=1):
        if key is None:
            raise _NotKeyError('Invalid parameter')
        observer = _Observer(key, auto_reset=auto_reset, keep=keep)
        self.nodes.append(observer)
        return

    def add_classical_observer(self, sites, opname, name=None):
        observer = _ClassicalObserver(sites, opname, name=name)
        self.nodes.append(observer)

    def _deserialize_result1d(self, result_data):
        # some deserialisation action decealised_result1d
        return result_data.get('message')

    @property
    def size(self):
        return self.L

    def clear(self):
        del self.nodes[:]

    def run(self):
        if not isinstance(self.states, QState):
            raise TypeError('State must be type QState.')
        # request authentication from server
        session = requests.session()
        headers = {
            'Accept': 'application/json',
            'Content-Type': 'application/json',
            'Authorization': '%s' % self.auth.token
        }
        packet = {'state': self.states.serialize(), 'nodes': self.serialize()}

        if self.auth.port == 80:
            if self.auth.ip.endswith('.com'):
                result = session.post(
                    url='https://%s/api/v1/circuit/result/' % self.auth.ip,
                    data=json.dumps(packet),
                    headers=headers
                )
            else:
                result = session.post(
                    url='https://%s/api/v1/circuit/result/' % self.auth.ip,
                    data=json.dumps(packet),
                    headers=headers
                )
        else:
            result = session.post(
                url='http://%s:%s/api/v1/circuit/result/' % (
                    self.auth.ip, self.auth.port),
                data=json.dumps(packet),
                headers=headers
            )
        if result.status_code == 200:
            result_data = eval(result.content.decode())
            return self._deserialize_result1d(result_data)
            # result = self._deserialize_result1d(result_data)
            # json_str = json.dumps(result)  # json str
            # return json_str
            # result2 = json.loads(json_str)
            # print(type(result2))

            # ans = result2['values']
            # return ans
        if result.status_code == 401:
            raise _RequestError('User Authentication Failed')
        result_data = eval(result.content.decode())
        raise _RequestError(result_data.get('message'))
