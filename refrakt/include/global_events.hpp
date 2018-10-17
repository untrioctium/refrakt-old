#include <set>
#include <vector>
#include <functional>

namespace refrakt {

	namespace detail {
		template <typename T> 
		struct has_accumulate_impl{
		private:
			template <typename C> static std::true_type test(decltype(&C::accumulate)) { return std::true_type; }
			template <typename C> static std::false_type test(...) { return std::false_type; }

		public:
			static constexpr bool value = decltype(test<T>(0))::value;
		};

		template <typename T>
		constexpr bool has_accumulate = has_accumulate_impl<T>::value;

		template<typename T>
		struct function_traits;

		template<typename R, typename ...Args>
		struct function_traits<R(Args...)>
		{
			static const size_t nargs = sizeof...(Args);

			typedef R result_type;

			template <std::size_t Index>
			using arg = typename std::tuple_element<
				Index,
				std::tuple<Args...>
			>::type;

		};

	}

	template <typename Event, typename Child, typename... Args>
	struct global_event_base {
		struct tag {
		private:
			tag() {} // only fire() can raise an event on an observer
			friend Child;
		};

		template<typename T, typename Enable = void>
		struct observer_impl {
			observer_impl() { observers().insert(this); }
			~observer_impl() { observers().erase(this); }

			virtual void on_notify(tag, Args... args) = 0;
		};

		template<typename T>
		struct observer_impl<T, typename std::enable_if<detail::has_accumulate<T>>::type> {
			observer_impl() { observers().insert(this); }
			~observer_impl() { observers().erase(this); }

			typedef typename detail::function_traits<decltype(T::accumulate)> accumulator_traits;
			virtual typename accumulator_traits::template arg<0>::value_type on_notify(tag, Args... args)  = 0;
		};

		using observer = observer_impl<Event>;

	protected:
		friend observer;

		static std::set<observer*>& observers() {
			static std::set<observer*> o;
			return o;
		}
	};

	template <typename Event, typename... Args>
	struct public_global_event : 
		global_event_base<Event, public_global_event<Event, Args...>, Args...> {
		using parent = global_event_base<Event, public_global_event, Args...>;

		static auto fire(Args... args){
			if constexpr(detail::has_accumulate<Event>) {
				typename detail::function_traits<decltype(Event::accumulate)>::arg<0> ret{};
				for (auto o : parent::observers())
					ret.push_back(o->on_notify(typename parent::tag{}, std::forward<Args>(args)...));

				return Event::accumulate(ret);
			}
			else {
				for (auto o : parent::observers())
					o->on_notify(typename parent::tag{}, std::forward<Args>(args)...);
			}
		}
	};

	template <typename Event, class Owner, typename... Args>
	struct private_global_event : 
		global_event_base<Event, private_global_event<Event, Args...>, Args...> {
	protected:
		using parent = global_event_base<Event, private_global_event, Args...>;
		friend Owner;
		static void fire(Args... args) {
			for (auto o : parent::observers())
				o->on_notify(typename parent::tag{}, std::forward<Args>(args)...);
		}
	};
}

namespace refrakt::events {
	struct gl_was_reset : public public_global_event<gl_was_reset> {};
	struct gl_calc_vram_usage : public public_global_event<gl_calc_vram_usage> {
		static std::size_t accumulate(std::vector<std::size_t> v);
	};
	struct gl_collect_garbage : public public_global_event<gl_collect_garbage> {};
}
