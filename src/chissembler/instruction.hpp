#pragma once

#include "target.hpp"
#include <common/visitor.hpp>
#include <gsl/gsl>
#include <memory>
#include <vector>
#include "emitter_state.hpp"

namespace instruction {
    class BasicInstruction;
}

using Instruction = std::unique_ptr<instruction::BasicInstruction>;

namespace instruction {

    class BasicInstruction {
    public:
        virtual ~BasicInstruction() = default;
        virtual void append(EmitterState& state) const = 0;
    };

    class Label final : public BasicInstruction {
    private:
        std::string m_name;

    public:
        explicit Label(std::string name) : m_name{ std::move(name) } { }
        void append(EmitterState& state) const override;
    };

    class Copy final : public BasicInstruction {
    private:
        Target m_source;
        Target m_destination;

    public:
        Copy(Target const& source, Target const& destination) : m_source{ source }, m_destination{ destination } { }
        void append(EmitterState& state) const override;
    };

    class Add final : public BasicInstruction {
    private:
        Target m_source;
        Target m_destination;

    public:
        Add(Target const& source, Target const& destination) : m_source{ source }, m_destination{ destination } { }
        void append(EmitterState& state) const override;
    };

} // namespace instruction
