#include "chissembler.hpp"

#include "emitter.hpp"
#include "lexer.hpp"
#include "source_location.hpp"
#include <common/types.hpp>


namespace chissembler {

    class State final : public EmitterState {
    private:
        std::vector<std::byte> m_machine_code;
        u16 m_address = 0x200;
        std::unordered_map<std::string, u16> m_labels;

    public:
        [[nodiscard]] std::vector<std::byte> move_out_machine_code() && {
            return std::move(m_machine_code);
        }

        [[nodiscard]] std::vector<std::byte>& machine_code() override {
            return m_machine_code;
        }

        [[nodiscard]] u16 address() const override {
            return m_address;
        }

        void advance_adress() override {
            m_address += 2;
        }

        [[nodiscard]] std::unordered_map<std::string, u16>& labels() override {
            return m_labels;
        }
    };

    [[nodiscard]] std::vector<std::byte> assemble(std::string_view const filename, std::string_view const source) {
        auto const tokens = Lexer::tokenize(filename, source);
        auto const instructions = Emitter::emit(tokens);
        auto state = State{};
        for (auto const& instruction : instructions) {
            instruction->append(state);
        }
        std::cout << "Labels:\n";
        for (auto&& [name, address] : state.labels()) {
            std::cout << name << "@0x" << std::hex << address << '\n';
        }
        return std::move(state).move_out_machine_code();
    }

} // namespace chissembler
