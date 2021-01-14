"""
This module provides a wrapper around the core envoy_engine infrastructure so that it plays nicely
gevent. In particular, it defines:

    - GeventEngineBuilder, which moves on_engine_running callbacks to the main thread and produces
      a GeventEngine.

    - GeventEngine, which produces a GeventStremClient.

    - GeventStreamClient, which produces a GeventStreamPrototype.

    - GeventStreamPrototype, which moves all HTTP callbacks to the main thread.

If you're using gevent, this module is highly suggested, otherwise you have to perform your own
synchronization while moving work across threads.
"""
import functools
import threading

import gevent
from gevent.pool import Group

from library.python import envoy_engine


class GeventEngineBuilder(envoy_engine.EngineBuilder):
    def __init__(self):
        super().__init__()
        self.executor = GeventExecutor()

    def set_on_engine_running(self, on_engine_running: Callable[[], None]) -> envoy_mobile.EngineBuilder:
        super().set_on_engine_running(self.executor.make_gevent(on_engine_running))
        return self

    def build(self) -> GeventEngine:
        return GeventEngine(super().build())


class GeventEngine(envoy_engine.Engine):
    def __init__(self, engine: envoy_engine.Engine):
        super().__init__(engine)

    def stream_client(self) -> envoy_mobile.StreamClient:
        return GeventStreamClient(super().stream_client())


class GeventStreamClient(envoy_engine.StreamClient):
    def __init__(self, stream_client: envoy_engine.StreamClient):
        super().__init__(stream_client)

    def new_stream_prototype(self) -> envoy_mobile.StreamPrototype:
        return GeventStreamPrototype(super().new_stream_prototype())


class GeventStreamPrototype(envoy_engine.StreamPrototype):
    def __init__(self, stream_prototype: envoy_engine.StreamPrototype):
        super().__init__(stream_prototype)
        self.executor = GeventExecutor()

    def set_on_headers(self, on_headers: Callable) -> enoy_mobile.StreamPrototype:
        super().set_on_headers(self.executor.make_gevent(on_headers))
        return self

    def set_on_data(self, on_data: Callable) -> enoy_mobile.StreamPrototype:
        super().set_on_data(self.executor.make_gevent(on_data))
        return self

    def set_on_trailers(self, on_trailers: Callable) -> enoy_mobile.StreamPrototype:
        super().set_on_trailers(self.executor.make_gevent(on_trailers))
        return self

    def set_on_error(self, on_error: Callable) -> enoy_mobile.StreamPrototype:
        super().set_on_error(self.executor.make_gevent(on_error))
        return self

    def set_on_complete(self, on_complete: Callable) -> enoy_mobile.StreamPrototype:
        super().set_on_complete(self.executor.make_gevent(on_complete))
        return self

    def set_on_cancel(self, on_cancel: Callable) -> enoy_mobile.StreamPrototype:
        super().set_on_cancel(self.executor.make_gevent(on_cancel))
        return self


class GeventExecutor():
    def __init__(self):
        self.group = Group()
        # TODO: actually write down the type for this
        self.channel = ThreadsafeChannel()  # type: ignore
        self.spawn_work_greenlet = gevent.spawn(self._spawn_work)

    def __del__(self):
        self.spawn_work_greenlet.kill()

    def make_gevent(self, func: Callable):
        @functools.wraps(func)
        def wrapper(*args, **kwargs):
            self.channel.put((func, args, kwargs))
        return wrapper

    def _spawn_work(self):
        while True:
            (func, args, kwargs) = self.channel.get()
            self.group.spawn(func, *args, **kwargs)


class ThreadsafeChannel(Generic[T]):
    def __init__(self):
        self.hub = gevent.get_hub()
        self.watcher = self.hub.loop.async_()
        self.lock = threading.Lock()
        self.values: List[T] = []

    def put(self, value: T) -> None:
        with self.lock:
            self.values.append(value)
            self.watcher.send()

    def get(self) -> T:
        self.lock.acquire()
        while len(self.values) == 0:
            self.lock.release()
            self.hub.wait(self.watcher)
            self.lock.acquire()

        value: T = self.values.pop(0)
        self.lock.release()
        return value
