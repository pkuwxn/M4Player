## 编译发行版（release）

暂时只在 32 位操作系统下编译通过。

安装必要的工具与开发库：
```sh
sudo apt-get install yasm
sudo apt-get install libpulse-dev
```

解压 “M4Player.tar.gz” 至任意目录（如主目录）:
```sh
tar -xzf M4Player.tar.gz
```

编译 wxWdigets：
```sh
export CXXFLAGS="-ffunction-sections -fdata-sections"
export LDFLAGS="-Wl,--gc-sections"
cd wx302
mkdir buildgtk
cd buildgtk
../configure --disable-debug --disable-debug_info --disable-debug_gdb --disable-shared
make
sudo make install
```

编译 FFmpeg：
```sh
export CXXFLAGS="-ffunction-sections -fdata-sections"
export LDFLAGS="-Wl,--gc-sections"
cd ../../FFmpeg-2.8.3
./configure \
--disable-debug --extra-version=static --disable-shared --enable-static --extra-cflags=--static \
--disable-programs \
--disable-doc \
--disable-devices \
--disable-network \
--disable-protocols --enable-protocol=file \
--enable-gpl --enable-version3 --enable-nonfree \
--disable-filters \
--disable-parsers --enable-parser=aac*,cook,mpegaudio,vorbis \
--disable-bsfs \
--disable-muxers --disable-encoders \
--disable-demuxers --enable-demuxer=aac,amr,ape,asf,flac,mp3,ogg,pcm*,rm,wav,xwma \
--disable-decoders --enable-decoder=aac,ape,amr*,cook,flac,mp1,mp2,mp3*,wma*,ra_*,vorbis
make
sudo make install
```

编译 M4Player：
```sh
cd ../M4Player
git pull origin
./GenMakefile.py
make config=release
cp bin/Release/*.so ../Assets/Plugins
cp bin/Release/m4 ../Assets
```

运行程序：
```sh
../Assets/m4
```

打包（可选）：
```sh
./Pack.sh
```

## 将千千静听的皮肤转换为 M4Player 支持的格式

项目附带了一个名为 TT2OO 的工具软件将千千静听的皮肤转换为 M4Player 可用的格式。
编译成功后，其可执行文件位于 Build/bin/Release/TT2OO 处。

千千静听的皮肤可在 <http://music.baidu.com/pc/skin_____.html> 或其他地方下载。
皮肤文件的后缀名为“.skn”，其真实格式是一个“.zip”压缩包。
假设它的文件名为“TT.skn”，
那么使用压缩包管理器将它解压到 M4Player 的安装目录下的Skins 子目录里。
“TT.skn”解压后的路径可能为 ~/M4Player/Skins/TT。
须保证 TT 子目录下存在名为 Skin.xml 的文件。

执行 TT2OO，会提示输入要转换的皮肤文件夹，输入“~/M4Player/Skins/TT”，
假如一切顺利则会输出“转换成功”，
且重启 M4Player 后即可在皮肤列表里看到新加入的皮肤。