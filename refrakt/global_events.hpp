#include <set>

namespace refrakt {
	template <typename Event, typename... Args>
	struct global_event_base {
		struct tag {
		};

		struct observer {
			observer() { observers().insert(this); }
			~observer() { observers().erase(this); }

			virtual void on_notify(tag, Args... args) = 0;
		};

	protected:
		friend observer;

		static std::set<observer*>& observers()
		{
			static std::set<observer*> o;
			return o;
		}
	};

	template <typename Event, typename... Args>
	struct public_global_event : global_event_base<Event, Args...> {
		using parent = global_event_base<Event, Args...>;

		static void fire(Args... args)
		{
			for (auto o : parent::observers())
				o->on_notify(typename parent::tag{}, std::forward<Args>(args)...);
		}
	};

	template <typename Event, class Owner, typename... Args>
	struct private_global_event : global_event_base<Event, Args...> {
	protected:
		using parent = global_event_base<Event, Args...>;
		friend Owner;
		static void fire(Args... args)
		{
			for (auto o : parent::observers())
				o->on_notify(typename parent::tag{}, std::forward<Args>(args)...);
		}
	};
}

namespace refrakt::events {
	struct gl_reset : public public_global_event<gl_reset> {};
	struct gl_calc_vram_usage : public public_global_event<gl_calc_vram_usage, std::size_t&> {};
}