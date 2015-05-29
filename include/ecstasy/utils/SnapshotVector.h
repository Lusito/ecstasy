#pragma once

#include <algorithm>
#include <vector>

namespace ECS {
	template<typename T>
	class SnapshotVector {
	private:
		enum class ActionType {
			ADD,
			REMOVE,
			REMOVE_ALL
		};
		struct Action {
			Action(ActionType type, T value) : type(type), value(value) {}
			ActionType type;
			T value;
		};
		bool blocked = false;
		std::vector<T> values;
		std::vector<Action> delayedActions;
		
	public:
		virtual ~SnapshotVector() {}

		const std::vector<T> &getValues() const {
			return values;
		}
		void block() {
			blocked = true;
		}
		void unblock() {
			blocked = false;
			if(!delayedActions.empty()) {
				for (Action &action: delayedActions) {
					switch(action.type) {
					case ActionType::ADD:
						addInternal(action.value);
						break;
					case ActionType::REMOVE:
						removeInternal(action.value);
						break;
					case ActionType::REMOVE_ALL:
						while(!values.empty())
							removeInternal(values.back());
						break;
					}
				}
				delayedActions.clear();
			}
		}
		
		void add(T value) {
			if(blocked)
				delayedActions.push_back(Action(ActionType::ADD, value));
			else
				addInternal(value);
		}

		void remove (T value) {
			if(blocked)
				delayedActions.push_back(Action(ActionType::REMOVE, value));
			else
				removeInternal(value);
		}

		/** Removes all Receivers attached to this {@link Signal}. */
		void removeAll() {
			if(blocked)
				delayedActions.push_back(Action(ActionType::REMOVE_ALL, nullptr));
			else {
				while(!values.empty())
					removeInternal(values.back());
			}
		}
		
	protected:
		virtual void addInternal(T value) {
			values.push_back(value);
		}
		virtual void removeInternal(T value) {
			values.erase(std::remove(values.begin(), values.end(), value), values.end());
		}
	};
}
