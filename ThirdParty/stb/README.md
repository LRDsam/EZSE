# stb_image

`stb_image.h` from https://github.com/nothings/stb, kept in git as is (not changed).

- Version: v2.30 (from the first line of the header)
- Commit: 003501f6d7a8668ce8b6d875e34dd97ad0d5d5b5
- Downloaded from: https://raw.githubusercontent.com/nothings/stb/003501f6d7a8668ce8b6d875e34dd97ad0d5d5b5/stb_image.h
- License: public domain or MIT, whichever you prefer (see the end of `stb_image.h`)

Used only in `Engine/src/Resources/Image.cpp`, which defines `STB_IMAGE_IMPLEMENTATION` and
`STBI_NO_STDIO` (images are decoded from memory; the engine reads the files itself).
