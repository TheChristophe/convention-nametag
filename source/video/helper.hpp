#ifndef CONVENTION_NAMETAG_HELPER_HPP
#define CONVENTION_NAMETAG_HELPER_HPP

#include <filesystem>
#include <optional>

std::optional<double> getVideoDuration(const std::filesystem::path &fileName);

#endif // CONVENTION_NAMETAG_HELPER_HPP
