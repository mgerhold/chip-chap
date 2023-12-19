#include "chissembler.hpp"

#include "emitter.hpp"
#include "lexer.hpp"
#include "source_location.hpp"

#include <common/types.hpp>
#include <iomanip>


namespace chissembler {

    class State final : public EmitterState {
    private:
        std::vector<std::byte> m_machine_code;
        u16 m_address = 0x200;
        std::unordered_map<std::string, u16> m_labels;
        std::vector<AddressPlaceholder> m_address_placeholders;

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

        [[nodiscard]] std::vector<AddressPlaceholder>& address_placeholders() override {
            return m_address_placeholders;
        }

        [[nodiscard]] std::unordered_map<std::string, u16>& labels() override {
            return m_labels;
        }
    };

    static void apply_address_placeholders(EmitterState& state) {
        for (auto const& placeholder : state.address_placeholders()) {
            auto const jump_target = visit(
                    placeholder.target,
                    [](u16 const addr) { return addr; },
                    [](std::tuple<u16, V0Offset> const& address_with_offset) {
                        return std::get<0>(address_with_offset);
                    },
                    [&](std::string const& label_name) {
                        auto const find_iterator = state.labels().find(label_name);
                        if (find_iterator == state.labels().cend()) {
                            // todo: include source location in error message
                            throw EmitterError{ std::format("unknown label '{}'", label_name) };
                        }
                        return find_iterator->second;
                    },
                    [&](std::tuple<std::string, V0Offset> const& label_name_with_offset) {
                        auto const find_iterator = state.labels().find(std::get<0>(label_name_with_offset));
                        if (find_iterator == state.labels().cend()) {
                            // todo: include source location in error message
                            throw EmitterError{
                                std::format("unknown label '{}'", std::get<0>(label_name_with_offset))
                            };
                        }
                        return find_iterator->second;
                    }
            );
            assert((jump_target & 0xF000) == 0);
            auto const msb = state.machine_code().at(placeholder.address - 0x200);
            auto const lsb = state.machine_code().at(placeholder.address - 0x200 + 1);
            state.machine_code().at(placeholder.address - 0x200) =
                    static_cast<std::byte>(static_cast<int>(msb) | (jump_target >> 8));
            state.machine_code().at(placeholder.address - 0x200 + 1) =
                    static_cast<std::byte>(static_cast<int>(lsb) | (jump_target & 0xFF));
        }
    }

    [[nodiscard]] std::vector<std::byte> assemble(std::string_view const filename, std::string_view const source) {
        auto const tokens = Lexer::tokenize(filename, source);
        auto const instructions = Emitter::emit(tokens);
        auto state = State{};
        for (auto const& instruction : instructions) {
            instruction->append(state);
        }
        std::cout << std::hex << std::uppercase;
        std::cout << "Labels:\n";
        for (auto&& [name, address] : state.labels()) {
            std::cout << "    " << name << " @ 0x" << address << '\n';
        }
        std::cout << "Placeholders:\n";
        for (auto const& placeholder : state.address_placeholders()) {
            std::cout << "    0x" << placeholder.address << " <- ";
            visit(
                    placeholder.target,
                    [&](u16 const address) { std::cout << "0x" << address; },
                    [&](std::tuple<u16, V0Offset> const& address_with_offset) {
                        auto const& [address, register_] = address_with_offset;
                        std::cout << "0x" << address << " + V0";
                    },
                    [&](std::string const& label_name) { std::cout << label_name; },
                    [&](std::tuple<std::string, V0Offset> const& label_with_offset) {
                        auto const& [label_name, register_] = label_with_offset;
                        std::cout << label_name << " + V0";
                    }
            );
            std::cout << '\n';
        }

        apply_address_placeholders(state);

        std::cout << "Binary:\n";
        auto const previous_fill = std::cout.fill('0');
        for (usize i = 0, address = 0x200; i < state.machine_code().size(); ++i, address += 2) {
            if ((i / 2 % 8) == 0 and i % 2 == 0) {
                std::cout << "    0x" << address << ": ";
            }
            std::cout << std::setw(2) << static_cast<int>(state.machine_code().at(i));
            if ((i / 2 + 1) % 8 == 0 and not(i % 2) == 0) {
                std::cout << '\n';
                continue;
            }
            if (i % 2 == 1) {
                std::cout << ' ';
            }
        }
        std::cout << std::setfill(previous_fill);
        return std::move(state).move_out_machine_code();
    }

} // namespace chissembler
