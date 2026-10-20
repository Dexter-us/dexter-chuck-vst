# Third-party notices

## This project and commercial use

The original Dexter U.S. Chuck VST source in this repository is licensed under AGPL-3.0-only (see LICENSE). The AGPL permits commercial use, paid copies, and paid services. It requires recipients to retain the same freedoms and to receive corresponding source in the situations described by the license. It does not grant exclusive control over redistribution.

## JUCE 9.0.4

This project uses JUCE 9.0.4 at commit 3fb5cbc0340758596dac39f7c8de47d562b4896e under JUCE's AGPLv3 option. JUCE's upstream license summary is in `third_party/licenses/JUCE-LICENSE.md`, and its complete SPDX software bill of materials (including bundled dependency versions and license expressions) is in `third_party/licenses/JUCE.spdx.json`. License texts available in the pinned JUCE source for bundled codecs, graphics, compression, and VST3 components are copied into `third_party/licenses/`; the SBOM remains the complete inventory, including optional components not linked by this build.

JUCE's bundled JPEG implementation includes Independent JPEG Group (IJG) code. Its upstream acknowledgement note is retained in `third_party/licenses/juce-graphics/jpeg-JUCE_UPSTREAM.txt`.

## ChucK 1.5.5.8

This project uses ChucK at commit 9d36c8dd7f45a75f375adff781f4cf7deb0ad910 under its MIT license option. The copyright and permission notice are in `third_party/licenses/ChucK-MIT.txt`. ChucK is also offered under GPL-2.0-or-later upstream; this project selects MIT.

## VST3 SDK

The VST3 SDK 3.8.0 is included by JUCE and is licensed under MIT. Its license is in `third_party/licenses/VST3-SDK-MIT.txt`.

## Other dependencies

The pinned JUCE SPDX bill of materials lists the dependency inventory and SPDX license expressions for the full JUCE source distribution; some listed components are optional and are not linked by this VST3-only build. Copyright and license notices for the compiled bundled codec, graphics, and compression libraries are preserved in the license files included here and in their upstream source. No Chuck Racks application source or Sonic Pi VST source is included in this product.
