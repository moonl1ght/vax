#pragma once

#include <utility>

namespace vax {
template <typename T> class state_tracker final {
  public:
    state_tracker(T object)
        : _object(object)
        , _isDirty(true) {}

    ~state_tracker() = default;

    void set_dirty(bool isDirty) { _isDirty = isDirty; }

    bool is_dirty() const { return _isDirty; }

    void clear_dirty() { _isDirty = false; }

    void mark_as_dirty() { _isDirty = true; }

    const T& get() const { return _object; }

    const T& get_and_clear_dirty() {
        _isDirty = false;
        return _object;
    }

    template <typename Updater> void update(Updater&& updater) {
        std::forward<Updater>(updater)(_object);
        _isDirty = true;
    }

    template <typename Updater> bool update_if_changed(Updater&& updater) {
        T temp = _object;
        std::forward<Updater>(updater)(temp);
        if (_object != temp) {
            _object = std::move(temp);
            _isDirty = true;
            return true;
        }
        return false;
    }

    bool set(T&& value) {
        if (_object != value) {
            _object = std::move(value);
            _isDirty = true;
            return true;
        }
        return false;
    }

  private:
    bool _isDirty;
    T _object;
};
} // namespace vax