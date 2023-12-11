#pragma once

#include "target.hpp"
#include <common/visitor.hpp>
#include <gsl/gsl>
#include <memory>
#include <vector>

namespace instruction {
    class BasicInstruction;
}

using Instruction = std::unique_ptr<instruction::BasicInstruction>;

namespace instruction {

    class BasicInstruction {
    public:
        virtual ~BasicInstruction() = default;
        virtual void append(std::vector<std::byte>& machine_code) const = 0;
    };

    class Copy final : public BasicInstruction {
    private:
        Target m_source;
        Target m_destination;

    public:
        Copy(Target const& source, Target const& destination) : m_source{ source }, m_destination{ destination } { }
        void append(std::vector<std::byte>& machine_code) const override;
    };

    class Add final : public BasicInstruction {
    private:
        Target m_source;
        Target m_destination;

    public:
        Add(Target const& source, Target const& destination) : m_source{ source }, m_destination{ destination } { }
        void append(std::vector<std::byte>& machine_code) const override;
    };

} // namespace instruction
