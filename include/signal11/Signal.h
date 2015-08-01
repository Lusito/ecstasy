// CC0 Public Domain: http://creativecommons.org/publicdomain/zero/1.0/
#pragma once

#include <functional>
#include <vector>
#include <assert.h>
#include <memory>
#include <algorithm>

namespace Signal11 {

	class ConnectionRef;

	namespace Lib {

		/// ProtoSignal is the template implementation for callback list.
		template<typename,typename> class ProtoSignal;   // undefined

		/// CollectorInvocation invokes signal handlers differently depending on return type.
		template<typename,typename> class CollectorInvocation;

		/// CollectorLast returns the result of the last signal handler from a signal emission.
		template<typename Result>
		class CollectorLast {
		public:
			typedef Result CollectorResult;

			explicit CollectorLast() {}

			inline bool operator()(Result r) {
				_last = r;
				return true;
			}
			
			CollectorResult result() {
				return _last;
			}

		private:
			Result _last;
		};

		/// CollectorDefault implements the default signal handler collection behaviour.
		template<typename Result>
		class CollectorDefault : public CollectorLast<Result> {};

		/// CollectorDefault specialisation for signals with void return type.
		template<>
		class CollectorDefault<void> {
		public:
			typedef void CollectorResult;

			void result() {}
			inline bool operator() (void) {
				return true;
			}
		};

		/// CollectorInvocation specialisation for regular signals.
		template<class Collector, class R, class... Args>
		class CollectorInvocation<Collector, R (Args...)> {
		public:
			inline bool	invoke(Collector &collector, const std::function<R (Args...)> &callback, Args... args) {
				return collector(callback(args...));
			}
		};

		/// CollectorInvocation specialisation for signals with void return type.
		template<class Collector, class... Args>
		struct CollectorInvocation<Collector, void (Args...)> {
			inline bool invoke(Collector &collector, const std::function<void (Args...)> &callback, Args... args) {
				callback(args...);
				return collector();
			}
		};

		struct ProtoSignalLink {
			virtual bool removeSibling(ProtoSignalLink *link) = 0;

			bool isEnabled() const {
				return _enabled && !_tempDisabled && !_newLink;
			}

			void enable() {
				setEnabled(true);
			}

			void disable() {
				setEnabled(false);
			}

			void setEnabled(bool flag) {
				_enabled = flag;
			}

			void setTempDisabled(bool flag) {
				_tempDisabled = flag;
			}

		protected:
			bool _enabled = true;
			bool _tempDisabled = false;
			bool _newLink = false;
			template<typename, typename> friend class ProtoSignal;
		};
	} // namespace Lib

	class ConnectionRef {
	public:
		ConnectionRef(const std::shared_ptr<Lib::ProtoSignalLink> &head, Lib::ProtoSignalLink *link)
			: _link(link), _head(head) {
			assert((head == nullptr && link == nullptr) || (head != nullptr && link != nullptr));
		}

		ConnectionRef(ConnectionRef &&other)
			:_link(std::move(other._link)) {
			std::swap(_head, other._head);
			other._link = nullptr;
		}

		virtual ~ConnectionRef() {}

		ConnectionRef& operator=(ConnectionRef &&other) {
			std::swap(_link, other._link);
			std::swap(_head, other._head);

			return *this;
		}

		bool operator==(const ConnectionRef &other) {
			return _link == other._link;
		}

		bool disconnect() {
			auto headPtr = _head.lock();
			if(headPtr != nullptr) {
				auto temp = _link;
				_link = nullptr;
				return headPtr->removeSibling(temp);
			}

			return false;
		}

		bool isEnabled() const {
			return isValid() && _link->isEnabled();
		}

		void enable() {
			if(isValid())
				_link->enable();
		}

		void disable() {
			if(isValid())
				_link->disable();
		}

		void setEnabled(bool flag) {
			if(isValid())
				_link->setEnabled(flag);
		}

		bool isValid() const {
			return !_head.expired() && _link != nullptr;
		}

	protected:
		Lib::ProtoSignalLink *_link = nullptr;

	private:
		std::weak_ptr<Lib::ProtoSignalLink> _head;
		
	};

	class ScopedConnectionRef : public ConnectionRef {
	public:
		ScopedConnectionRef()
			:ConnectionRef(nullptr, nullptr) {}

		ScopedConnectionRef(ConnectionRef &&ref)
			:ConnectionRef(std::move(ref)) {}

		ScopedConnectionRef(ScopedConnectionRef &&other)
			:ConnectionRef(std::move(other)) {}

		~ScopedConnectionRef() {
			disconnect();
		}

		ScopedConnectionRef& operator=(ScopedConnectionRef &&other) {
			ConnectionRef::operator=(std::move(other));

			return *this;
		}

		bool operator==(const ScopedConnectionRef &other) {
			return ConnectionRef::operator==(other);
		}

		bool operator==(const ConnectionRef &other) {
			return ConnectionRef::operator==(other);
		}

		ConnectionRef release() {
			return std::move(static_cast<ConnectionRef&>(*this));
		}

	protected:
		friend class ConnectionScope;

		void setTempDisabled(bool flag) {
			_link->setTempDisabled(flag);
		}

	private:
		ScopedConnectionRef(const ScopedConnectionRef &other)
			:ConnectionRef(nullptr, nullptr) {}
		ScopedConnectionRef& operator=(const ScopedConnectionRef &other) { return *this; }
	};
	
	class ConnectionScope {
	public:
		ScopedConnectionRef& operator+=(ConnectionRef &&ref) {
			return insert(std::move(ref));
		}

		ScopedConnectionRef& operator+=(ScopedConnectionRef &&ref) {
			return insert(std::move(ref));
		}

		bool operator-=(const ConnectionRef &ref) {
			return remove(ref);
		}

		bool operator -=(const ScopedConnectionRef &ref) {
			return remove(ref);
		}

		ScopedConnectionRef& insert(ConnectionRef &&ref) {
			_connections.push_back(std::move(ref));
			return _connections.back();
		}

		ScopedConnectionRef& insert(ScopedConnectionRef &&ref) {
			_connections.push_back(std::move(ref));
			return _connections.back();
		}

		bool remove(const ConnectionRef &ref) {
			auto itr = std::find(std::begin(_connections), std::end(_connections), ref);

			if(itr != std::end(_connections)) {
				_connections.erase(itr);
				return true;
			}

			return false;
		}

		bool remove(const ScopedConnectionRef &ref) {
			auto itr = std::find(std::begin(_connections), std::end(_connections), ref);

			if(itr != std::end(_connections)) {
				_connections.erase(itr);
				return true;
			}

			return false;
		}

		bool remove(const ConnectionRef &ref, ScopedConnectionRef &ret) {
			auto itr = std::find(std::begin(_connections), std::end(_connections), ref);

			if(itr != std::end(_connections)) {
				ret = std::move(*itr);
				_connections.erase(itr);
				return true;
			}

			return false;
		}

		bool remove(const ScopedConnectionRef &ref, ScopedConnectionRef &ret) {
			auto itr = std::find(std::begin(_connections), std::end(_connections), ref);

			if(itr != std::end(_connections)) {
				ret = std::move(*itr);
				_connections.erase(itr);
				return true;
			}

			return false;
		}

		bool removeReleased(const ConnectionRef &ref, ConnectionRef &ret) {
			auto itr = std::find(std::begin(_connections), std::end(_connections), ref);

			if(itr != std::end(_connections)) {
				ret = itr->release();
				_connections.erase(itr);
				return true;
			}

			return false;
		}

		bool removeReleased(const ScopedConnectionRef &ref, ConnectionRef &ret) {
			auto itr = std::find(std::begin(_connections), std::end(_connections), ref);

			if(itr != std::end(_connections)) {
				ret = itr->release();
				_connections.erase(itr);
				return true;
			}

			return false;
		}

		bool isEnabled() const {
			return _enabled;
		}

		void enable() {
			setEnabled(true);
		}

		void disable() {
			setEnabled(false);
		}

		void setEnabled(bool flag) {
			if(flag && !_enabled || !flag && _enabled) {
				for(auto &connection : _connections) {
					connection.setTempDisabled(!flag);
				}
			}

			_enabled = flag;
		}

		void removeAll() {
			_connections.clear();
		}

	private:
		std::vector<ScopedConnectionRef> _connections;
		bool _enabled = true;
	};

	namespace Lib {
		/// ProtoSignal template specialised for the callback signature and collector.
		template<class Collector, class R, class... Args>
		class ProtoSignal<R (Args...), Collector> : private CollectorInvocation<Collector, R (Args...)> {
		protected:
			typedef std::function<R (Args...)> CallbackFunction;
			typedef typename CallbackFunction::result_type Result;
			typedef typename Collector::CollectorResult CollectorResult;

		private:
			/// SignalLink implements a doubly-linked ring with ref-counted nodes containing the signal handlers.
			struct SignalLink : public ProtoSignalLink {
				SignalLink *_next = nullptr;
				SignalLink *_prev = nullptr;
				CallbackFunction _callbackFunc;
				int _refCount = 1;
				int _priority;

				friend class ConnectionRef;

				explicit SignalLink(const CallbackFunction &callback, int priority=0)
					: _callbackFunc(callback), _priority(priority) {}

				~SignalLink() {
					assert(_refCount == 0);
				}

				void incref() {
					_refCount += 1;
					assert(_refCount > 0);
				}

				void decref(bool performDelete = true) {
					_refCount -= 1;
					
					if(_refCount == 0) {
						if(performDelete)
							delete this;
					} else {
						assert(_refCount > 0);
					}
				}

				void unlink() {
					_callbackFunc = nullptr;
					
					if(_next)
						_next->_prev = _prev;

					if(_prev)
						_prev->_next = _next;

					decref();
					// leave intact ->_next, ->_prev for stale iterators
				}

				SignalLink* addBefore(const CallbackFunction &callback, int priority) {
					auto *after = _prev;
					while (after != this) {
						if (after->_priority <= priority)
							break;
						after = after->_prev;
					}

					SignalLink *link = new SignalLink(callback, priority);
					link->_prev = after;
					link->_next = after->_next;
					after->_next = link;
					link->_next->_prev = link;

					static_assert(sizeof(link) == sizeof(size_t), "sizeof size_t");

					return link;
				}

				bool removeSibling(ProtoSignalLink *id) override {
					SignalLink *link = this->_next ? this->_next : this;

					for(; link != this; link = link->_next) {
						if(id == link) {
							link->unlink();     // deactivate and unlink sibling
							return true;
						}
					}
					
					return false;
				}
			};

			std::shared_ptr<SignalLink> _callbackRing; // linked ring of callback nodes
			bool _hasNewLinks = false;
			int _emitDepth = 0;
			
			void ensureRing() {
				if(!_callbackRing) {
					_callbackRing = std::make_shared<SignalLink>(CallbackFunction()); // refCount = 1
					_callbackRing->incref(); // refCount = 2, head of ring, can be deactivated but not removed
					_callbackRing->_next = _callbackRing.get(); // ring head initialization
					_callbackRing->_prev = _callbackRing.get(); // ring tail initialization
				}
			}

			ProtoSignal (const ProtoSignal&) {}
			ProtoSignal& operator=(const ProtoSignal&) {}

		public:
			/// ProtoSignal constructor, connects default callback if non-NULL.
			ProtoSignal(const CallbackFunction &method) {
				if(method != nullptr) {
					ensureRing();
					_callbackRing->_callbackFunc = method;
				}
			}

			ProtoSignal(ProtoSignal &&other)
				:_callbackRing(std::move(other._callbackRing)) {
				other._callbackRing = nullptr;
			}

			/// ProtoSignal destructor releases all resources associated with this signal.
			~ProtoSignal() {
				if (_callbackRing) {
					while(_callbackRing->_next != _callbackRing.get()) {
						_callbackRing->_next->unlink();
					}

					assert(_callbackRing->_refCount >= 2);
					_callbackRing->decref();
					_callbackRing->decref(false);
				}
			}

			ProtoSignal& operator=(ProtoSignal &&other) {
				std::swap(_callbackRing, other._callbackRing);

				return *this;
			}

			/// Operator to add a new function or lambda as signal handler, returns a handler connection ID.
			ConnectionRef connect(const CallbackFunction &callback) {
				return connect(0, callback);
			}
			ConnectionRef connect(int priority, const CallbackFunction &callback) {
				ensureRing();
				auto *link = _callbackRing->addBefore(callback, priority);
				if (_emitDepth > 0) {
					_hasNewLinks = true;
					link->_newLink = true;
				}
				return ConnectionRef(_callbackRing, link);
			}

			template<class T>
			ConnectionRef connect(T &object, R(T::*method)(Args...)) {
				return connect(0, object, method);
			}
			template<class T>
			ConnectionRef connect(int priority, T &object, R(T::*method)(Args...)) {
				CallbackFunction wrapper = [&object, method](Args&&... args) -> R {
					return (object.*method)(args...);
				};

				return connect(priority, wrapper);
			}

			template<class T>
			ConnectionRef connect(T *object, R(T::*method)(Args...)) {
				return connect(0, object, method);
			}
			template<class T>
			ConnectionRef connect(int priority, T *object, R(T::*method)(Args...)) {
				CallbackFunction wrapper = [object, method](Args&&... args) -> R {
					return (object->*method)(args...);
				};

				return connect(priority, wrapper);
			}

			/// Operator to remove a signal handler through it connection ID, returns if a handler was removed.
			bool disconnect(ConnectionRef &connectionRef) {
				return connectionRef.disconnect();
			}

			bool disconnect(SignalLink *link) {
				return _callbackRing ? _callbackRing->removeSibling(link) : false;
			}

			bool disconnect(ProtoSignalLink *link) {
				return disconnect((SignalLink*)link);
			}

			/// Emit a signal, i.e. invoke all its callbacks and collect return types with the Collector.
			CollectorResult emit(Args... args) {
				Collector collector;

				if (!_callbackRing)
					return collector.result();

				SignalLink *link = _callbackRing.get();
				link->incref();
				_emitDepth++;

				do {
					if(link->isEnabled() && link->_callbackFunc != nullptr) {
						const bool continueEmission = this->invoke(collector, link->_callbackFunc, args...);

						if(!continueEmission)
							break;
					}

					SignalLink *old = link;
					link = old->_next;
					link->incref();
					old->decref();
				}
				while (link != _callbackRing.get());
				
				link->decref();
				_emitDepth--;
				if (_hasNewLinks && _emitDepth == 0) {
					for (link = link->_next; link != _callbackRing.get(); link = link->_next)
						link->_newLink = false;
				}
				return collector.result();
			}
		};

	} // Lib
	// namespace Signal11

	/**
	* Signal is a template type providing an interface for arbitrary callback lists.
	* A signal type needs to be declared with the function signature of its callbacks,
	* and optionally a return result collector class type.
	* Signal callbacks can be added with operator+= to a signal and removed with operator-=, using
	* a callback connection ID return by operator+= as argument.
	* The callbacks of a signal are invoked with the emit() method and arguments according to the signature.
	* The result returned by emit() depends on the signal collector class. By default, the result of
	* the last callback is returned from emit(). Collectors can be implemented to accumulate callback
	* results or to halt a running emissions in correspondance to callback results.
	* The signal implementation is safe against recursion, so callbacks may be removed and
	* added during a signal emission and recursive emit() calls are also safe.
	* The overhead of an unused signal is intentionally kept very low, around the size of a single pointer.
	* Note that the Signal template types is non-copyable.
	*/
	template <typename SignalSignature, class Collector = Lib::CollectorDefault<typename std::function<SignalSignature>::result_type> >
	struct Signal final : Lib::ProtoSignal<SignalSignature, Collector> {
		typedef Lib::ProtoSignal<SignalSignature, Collector> ProtoSignal;
		typedef typename ProtoSignal::CallbackFunction CallbackFunction;

		/// Signal constructor, supports a default callback as argument.
		Signal(const CallbackFunction &method = CallbackFunction())
			:ProtoSignal(method) {}

		Signal(Signal &&other)
			:ProtoSignal(std::move(other)) {}

		Signal& operator=(Signal &&other) {
			ProtoSignal::operator=(std::move(other));
			return *this;
		}
	};

	/// Keep signal emissions going while all handlers return !0 (true).
	template<typename Result>
	struct CollectorUntil0 {
		typedef Result CollectorResult;

		explicit CollectorUntil0() {}

		const CollectorResult& result() {
			return _result;
		}

		inline bool	operator() (Result r) {
			_result = r;
			return _result ? true : false;
		}

	private:
		CollectorResult _result;
	};

	/// Keep signal emissions going while all handlers return 0 (false).
	template<typename Result>
	struct CollectorWhile0 {
		typedef Result CollectorResult;

		explicit CollectorWhile0() {}

		const CollectorResult& result() {
			return _result;
		}

		inline bool operator() (Result r) {
			_result = r;
			return _result ? false : true;
		}

	private:
		CollectorResult _result;
	};

	/// CollectorVector returns the result of the all signal handlers from a signal emission in a std::vector.
	template<typename Result>
	struct CollectorVector {
		typedef std::vector<Result> CollectorResult;

		const CollectorResult& result() {
			return _result;
		}

		inline bool	operator() (Result r) {
			_result.push_back (r);
			return true;
		}

	private:
		CollectorResult _result;
	};

} // Signal11

#ifdef USING_SIGNAL11
	using Signal11::ConnectionRef;
	using Signal11::ScopedConnectionRef;
	using Signal11::ConnectionScope;
	using Signal11::Signal;
	using Signal11::CollectorUntil0;
	using Signal11::CollectorWhile0;
	using Signal11::CollectorVector;
#endif
