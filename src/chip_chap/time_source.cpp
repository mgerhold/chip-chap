#include "time_source.hpp"

[[nodiscard]] double TimeSource::elapsed_seconds() const {
    return m_elapsed;
}

void TimeSource::advance(double const seconds) {
    m_elapsed += seconds;
}
