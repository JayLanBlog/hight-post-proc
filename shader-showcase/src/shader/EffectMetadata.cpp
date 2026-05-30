#include "shader/EffectMetadata.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstdio>

// ============================================================================
// Simple JSON parser - no external dependencies
// ============================================================================

namespace {

std::string Trim(const std::string& s) {
    size_t start = 0, end = s.size();
    while (start < end && (s[start] == ' ' || s[start] == '\t' || s[start] == '\n' || s[start] == '\r')) ++start;
    while (end > start && (s[end-1] == ' ' || s[end-1] == '\t' || s[end-1] == '\n' || s[end-1] == '\r')) --end;
    return s.substr(start, end - start);
}

std::string ReadFile(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) {
        fprintf(stderr, "[EffectMetadata] Cannot open: %s\n", path.c_str());
        return "";
    }
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

// Extract the first quoted string starting at/after pos.
// Returns the unquoted content and advances pos past the closing quote.
std::string ExtractString(const std::string& json, size_t& pos) {
    pos = json.find('"', pos);
    if (pos == std::string::npos) return "";
    size_t start = ++pos;
    while (pos < json.size()) {
        if (json[pos] == '\\') { pos += 2; continue; }
        if (json[pos] == '"') break;
        ++pos;
    }
    std::string result = json.substr(start, pos - start);
    ++pos; // skip closing quote
    return result;
}

// Extract a numeric value (int or float) and advance pos.
double ExtractNumber(const std::string& json, size_t& pos) {
    while (pos < json.size() && std::isspace(static_cast<unsigned char>(json[pos]))) ++pos;
    size_t start = pos;
    if (json[pos] == '-') ++pos;
    while (pos < json.size() && (std::isdigit(static_cast<unsigned char>(json[pos])) || json[pos] == '.')) ++pos;
    return std::stod(json.substr(start, pos - start));
}

// Extract a boolean value and advance pos.
bool ExtractBool(const std::string& json, size_t& pos) {
    while (pos < json.size() && std::isspace(static_cast<unsigned char>(json[pos]))) ++pos;
    if (json.compare(pos, 4, "true") == 0) { pos += 4; return true; }
    if (json.compare(pos, 5, "false") == 0) { pos += 5; return false; }
    return false;
}

// Extract an array of quoted strings.
std::vector<std::string> ExtractStringArray(const std::string& json, size_t& pos) {
    std::vector<std::string> result;
    pos = json.find('[', pos);
    if (pos == std::string::npos) return result;
    ++pos; // skip '['
    while (pos < json.size()) {
        if (json[pos] == ']') { ++pos; break; }
        if (json[pos] == ',' || std::isspace(static_cast<unsigned char>(json[pos]))) { ++pos; continue; }
        if (json[pos] == '"') {
            result.push_back(ExtractString(json, pos));
            continue;
        }
        ++pos;
    }
    return result;
}

// Find the value for a given key in a JSON object scope (between { and }).
// Returns the unquoted string value, or empty if the value is not a string.
std::string GetStringValue(const std::string& scope, const std::string& key) {
    std::string pattern = "\"" + key + "\"";
    size_t p = scope.find(pattern);
    if (p == std::string::npos) return "";
    p = scope.find(':', p + pattern.size());
    if (p == std::string::npos) return "";
    ++p;
    // Check that the value is actually a quoted string
    while (p < scope.size() && std::isspace(static_cast<unsigned char>(scope[p]))) ++p;
    if (p >= scope.size() || scope[p] != '"') return ""; // not a string value
    --p; // back up so ExtractString finds the quote
    return ExtractString(scope, p);
}

double GetNumberValue(const std::string& scope, const std::string& key) {
    std::string pattern = "\"" + key + "\"";
    size_t p = scope.find(pattern);
    if (p == std::string::npos) return 0.0;
    p = scope.find(':', p + pattern.size());
    if (p == std::string::npos) return 0.0;
    ++p; // skip ':'
    return ExtractNumber(scope, p);
}

bool GetBoolValue(const std::string& scope, const std::string& key) {
    std::string pattern = "\"" + key + "\"";
    size_t p = scope.find(pattern);
    if (p == std::string::npos) return false;
    p = scope.find(':', p + pattern.size());
    if (p == std::string::npos) return false;
    ++p; // skip ':'
    return ExtractBool(scope, p);
}

ParamType ParseParamType(const std::string& typeStr) {
    if (typeStr == "Float")  return ParamType::Float;
    if (typeStr == "Int")    return ParamType::Int;
    if (typeStr == "Bool")   return ParamType::Bool;
    if (typeStr == "Float2") return ParamType::Float2;
    if (typeStr == "Float3") return ParamType::Float3;
    if (typeStr == "Float4") return ParamType::Float4;
    if (typeStr == "Color")  return ParamType::Color;
    return ParamType::Float;
}

} // anonymous namespace

// ============================================================================
// Public API
// ============================================================================

EffectCard LoadEffectFromJson(const std::string& filepath) {
    EffectCard card;

    // Safety check: ensure file exists before reading
    FILE* test = fopen(filepath.c_str(), "rb");
    if (!test) {
        fprintf(stderr, "[EffectMetadata] File not found: %s\n", filepath.c_str());
        return card;
    }
    // Check file size, skip if too large (>100KB)
    fseek(test, 0, SEEK_END);
    long sz = ftell(test);
    fclose(test);
    if (sz <= 0 || sz > 102400) {
        fprintf(stderr, "[EffectMetadata] Invalid file size %ld for: %s\n", sz, filepath.c_str());
        return card;
    }

    std::string json = ReadFile(filepath);
    if (json.empty()) {
        fprintf(stderr, "[EffectMetadata] Empty or missing file: %s\n", filepath.c_str());
        return card;
    }

    // Remove C-style comments
    {
        std::string cleaned;
        cleaned.reserve(json.size());
        for (size_t i = 0; i < json.size(); ++i) {
            if (json[i] == '/' && i+1 < json.size()) {
                if (json[i+1] == '/') {
                    // line comment - skip to end of line
                    i += 2;
                    while (i < json.size() && json[i] != '\n') ++i;
                    if (i < json.size()) cleaned += '\n';
                    continue;
                }
                if (json[i+1] == '*') {
                    // block comment
                    i += 2;
                    while (i+1 < json.size() && !(json[i] == '*' && json[i+1] == '/')) ++i;
                    i += 1; // skip closing /
                    continue;
                }
            }
            cleaned += json[i];
        }
        json = std::move(cleaned);
    }

    // Top-level fields
    card.name        = GetStringValue(json, "name");
    card.category    = GetStringValue(json, "category");
    card.description = GetStringValue(json, "description");
    card.passes      = static_cast<int>(GetNumberValue(json, "passes"));
    if (card.passes < 1) card.passes = 1;

    // Parse params array
    size_t paramsStart = json.find("\"params\"");
    if (paramsStart != std::string::npos) {
        paramsStart = json.find('[', paramsStart);
        if (paramsStart != std::string::npos) {
            // Find matching ']'
            int bracketDepth = 0;
            size_t paramsEnd = paramsStart;
            for (size_t i = paramsStart; i < json.size(); ++i) {
                if (json[i] == '[') ++bracketDepth;
                else if (json[i] == ']') {
                    --bracketDepth;
                    if (bracketDepth == 0) { paramsEnd = i; break; }
                }
            }

            // Extract individual param objects
            size_t pos = paramsStart + 1;
            while (pos < paramsEnd) {
                // Find next '{'
                pos = json.find('{', pos);
                if (pos == std::string::npos || pos >= paramsEnd) break;

                // Find matching '}'
                int depth = 0;
                size_t objEnd = pos;
                for (size_t i = pos; i < paramsEnd; ++i) {
                    if (json[i] == '{') ++depth;
                    else if (json[i] == '}') {
                        --depth;
                        if (depth == 0) { objEnd = i; break; }
                    }
                }

                std::string paramScope = json.substr(pos, objEnd - pos + 1);

                ShaderParam param;
                param.name    = GetStringValue(paramScope, "name");
                param.label   = GetStringValue(paramScope, "label");
                param.type    = ParseParamType(GetStringValue(paramScope, "type"));
                param.minVal  = static_cast<float>(GetNumberValue(paramScope, "min"));
                param.maxVal  = static_cast<float>(GetNumberValue(paramScope, "max"));

                // "default" can be a number or an array of numbers
                // First try as a quoted string (array format or literal string)
                std::string defStr = GetStringValue(paramScope, "default");
                // Only use string value if it looks like a number (simple string defaults)
                if (!defStr.empty()) {
                    try {
                        param.defaultVal[0] = static_cast<float>(std::stod(defStr));
                    } catch (...) {
                        // Not a number string, fall through to numeric parsing
                        defStr.clear();
                    }
                }
                if (defStr.empty()) {
                    // Not a string value, try as a bare number
                    double defNum = GetNumberValue(paramScope, "default");
                    param.defaultVal[0] = static_cast<float>(defNum);
                }

                param.uiType = GetStringValue(paramScope, "ui_type");

                // comboOptions
                size_t comboPos = paramScope.find("\"combo_options\"");
                if (comboPos != std::string::npos) {
                    param.comboOptions = ExtractStringArray(paramScope, comboPos);
                }

                card.params.push_back(std::move(param));
                pos = objEnd + 1;
            }
        }
    }

    return card;
}
