## Building

### Prerequisites:

0. get meson from pip. Do not use ubuntu package, it is too old.
1. get godot:  https://docs.godotengine.org/en/stable/contributing/development/compiling/index.html
2. get clang 18+
   ```
   wget https://apt.llvm.org/llvm.sh
   chmod +x llvm.sh
   ./llvm.sh 18 all
   ```
   Without 'all' it will not install libc++


   To register it with update-alternatives on ubuntu, use this script:
   https://gist.github.com/junkdog/70231d6953592cd6f27def59fe19e50d

2. Build it with following options:
  scons platform=linuxbsd use_llvm=yes arch=x86_64 linker=lld cxxflags="-stdlib=libc++" linkflags="-stdlib=libc++"

3. Install ninja
   ```
   aptitude install ninja-build
   ```
   
### Building project

Make build dir
```
meson setup --native-file clang.native.ini --prefix ABSOLUTE/PATH build
```
where ABSOLUTE/PATH is absolute path to the following subfolder:
  ui/godot/project

Then build as usual
```
cd build
ninja .
```

Then install compiled library
```
cd build
meson install
```
