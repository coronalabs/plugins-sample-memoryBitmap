# TextureResourceExternal example

Simple demo of cross-platform bitmap texture.


Usage:

```
local memTex = require "plugin.memoryBitmap"


local tex = memTex.newTexture({width=100, height=100})

display.newImage( tex.filename , tex.baseDir, display.contentCenterX, display.contentCenterY )

for y=1, tex.height do
	for x=1, tex.width do
		tex:setPixel( x,y, 1,0,0,1 )
	end
end

tex:invalidate()
tex:releaseSelf()

```

Code is shared among all platforms and is in `shared/` directory

## Links

* [CoronaGraphics](https://docs.coronalabs.com/daily/native/C/CoronaGraphics.html) - library to used to push textures into corona
* [TextureResourceExternal](https://docs.coronalabs.com/daily/api/type/TextureResourceExternal/index.html) - texture type produced by plugins
* [Memory bitmap](https://marketplace.coronalabs.com/plugin/memory-bitmap) plugin on Corona Marketplace
* Plugin [documentation](https://docs.coronalabs.com/daily/plugin/memoryBitmap/index.html)

