#include "ChuckTokeniser.h"

namespace
{
constexpr int errorToken = 0;
constexpr int commentToken = 1;
constexpr int keywordToken = 2;
constexpr int audioTypeToken = 3;
constexpr int operatorToken = 4;
constexpr int identifierToken = 5;
constexpr int integerToken = 6;
constexpr int floatToken = 7;
constexpr int stringToken = 8;
constexpr int bracketToken = 9;
constexpr int punctuationToken = 10;

bool isIdentifierStart(juce::juce_wchar c)
{
    return juce::CharacterFunctions::isLetter(c) || c == '_';
}

bool isIdentifierBody(juce::juce_wchar c)
{
    return juce::CharacterFunctions::isLetterOrDigit(c) || c == '_';
}

void skipString(juce::CodeDocument::Iterator& source, juce::juce_wchar quote)
{
    source.skip();
    while (!source.isEOF())
    {
        const auto c = source.nextChar();
        if (c == '\\' && !source.isEOF())
            source.skip();
        else if (c == quote || c == '\n')
            break;
    }
}
}

bool ChuckTokeniser::isKeyword(const juce::String& word)
{
    static const juce::StringArray words {
        "if", "else", "while", "do", "until", "for", "break", "continue",
        "return", "switch", "repeat", "class", "extends", "public", "static",
        "pure", "this", "super", "interface", "implements", "protected",
        "private", "function", "fun", "spork", "const", "new", "now", "true",
        "false", "maybe", "null", "int", "float", "time", "dur", "polar",
        "complex", "void", "same", "global", "me"
    };
    return words.contains(word);
}

bool ChuckTokeniser::isAudioType(const juce::String& word)
{
    static const juce::StringArray words {
        "SinOsc", "TriOsc", "SawOsc", "SqrOsc", "PulseOsc", "Noise", "CNoise",
        "Impulse", "Step", "Gain", "Pan2", "ADSR", "Envelope", "LPF", "HPF",
        "BPF", "BRF", "BiQuad", "Delay", "DelayA", "DelayL", "Echo", "Chorus",
        "JCRev", "NRev", "SndBuf", "LiSa", "adc", "dac", "blackhole",
        "Std", "Math", "Event", "Shred", "Machine", "FileIO"
    };
    return words.contains(word);
}

int ChuckTokeniser::readNextToken(juce::CodeDocument::Iterator& source)
{
    source.skipWhitespace();
    const auto first = source.peekNextChar();
    if (first == 0)
        return errorToken;

    if (first == '/' && source.peekNextChar() != 0)
    {
        auto lookahead = source;
        lookahead.skip();
        if (lookahead.peekNextChar() == '/')
        {
            source.skipToEndOfLine();
            return commentToken;
        }
        if (lookahead.peekNextChar() == '*')
        {
            source.skip();
            source.skip();
            while (!source.isEOF())
            {
                const auto c = source.nextChar();
                if (c == '*' && source.peekNextChar() == '/')
                {
                    source.skip();
                    break;
                }
            }
            return commentToken;
        }
    }

    if (first == '"' || first == '\'')
    {
        skipString(source, first);
        return stringToken;
    }

    if (juce::CharacterFunctions::isDigit(first))
    {
        bool hasDot = false;
        while (juce::CharacterFunctions::isDigit(source.peekNextChar())
               || (!hasDot && source.peekNextChar() == '.'))
        {
            if (source.peekNextChar() == '.')
                hasDot = true;
            source.skip();
        }
        return hasDot ? floatToken : integerToken;
    }

    if (isIdentifierStart(first))
    {
        juce::String word;
        while (isIdentifierBody(source.peekNextChar()))
            word += source.nextChar();
        if (isKeyword(word))
            return keywordToken;
        if (isAudioType(word))
            return audioTypeToken;
        return identifierToken;
    }

    source.skip();
    if (first == '(' || first == ')' || first == '{' || first == '}'
        || first == '[' || first == ']')
        return bracketToken;
    if (first == ';' || first == ',' || first == ':' || first == '.')
        return punctuationToken;
    if (juce::String("+-*/%=!<>|&^~@").containsChar(first))
        return operatorToken;
    return errorToken;
}

juce::CodeEditorComponent::ColourScheme ChuckTokeniser::getDefaultColourScheme()
{
    juce::CodeEditorComponent::ColourScheme scheme;
    scheme.set("Error", juce::Colour(0xffff6470));
    scheme.set("Comment", juce::Colour(0xff7d8c90));
    scheme.set("Keyword", juce::Colour(0xffc792ea));
    scheme.set("Audio type", juce::Colour(0xff82aaff));
    scheme.set("Operator", juce::Colour(0xffffcb6b));
    scheme.set("Identifier", juce::Colour(0xffe4e9ee));
    scheme.set("Integer", juce::Colour(0xfff78c6c));
    scheme.set("Float", juce::Colour(0xfff78c6c));
    scheme.set("String", juce::Colour(0xffc3e88d));
    scheme.set("Bracket", juce::Colour(0xff89ddff));
    scheme.set("Punctuation", juce::Colour(0xff89ddff));
    return scheme;
}
