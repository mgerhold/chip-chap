#pragma once

#include "emitter_state.hpp"
#include "target.hpp"
#include "token.hpp"

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
        virtual void append(EmitterState& state) const = 0;
    };

    class Label final : public BasicInstruction {
    private:
        Token m_label_token;

    public:
        explicit Label(Token const& label_token) : m_label_token{ label_token } { }
        void append(EmitterState& state) const override;
    };

    class Jump final : public BasicInstruction {
    private:
        JumpTarget m_target;

    public:
        explicit Jump(JumpTarget target) : m_target{ std::move(target) } { }
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
