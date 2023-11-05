# Boost.Scope

Boost.Scope provides a number of scope guard utilities described in [C++ Extensions for Library Fundamentals, Version 3](https://github.com/cplusplus/fundamentals-ts/releases/tag/n4908), Section 3.3 Scope guard support \[scopeguard\]. The implementation also provides a few non-standard extensions.

This library is currently proposed for review and potential inclusion into [Boost](https://www.boost.org/).

### Directories

* **doc** - QuickBook documentation sources
* **include** - Interface headers of Boost.Scope
* **test** - Boost.Scope unit tests

### Installation

#### Using Conan

````
git clone https://github.com/Lastique/scope
conan create scope/conan --build missing
````

This will build a boost_scope package using your default profile and put it
in the local Conan cache along with all direct and transitive dependencies.
Since Scope only depends on a few header-only Boost libraries, you can
save some time by requesting header-only Boost:

```
conan create scope/conan -o 'boost*:header_only=True' --build missing
````
Following one of those approaches you can use the package as usual. For
example, using a `conanfile.txt`:
```
[requires]
boost_scope/1.0.0
````

### More information

* Read the [documentation](https://lastique.github.io/scope/libs/scope/doc/html/index.html).
* [Report bugs](https://github.com/Lastique/scope/issues/new). Be sure to mention Boost version, platform and compiler you're using. A small compilable code sample to reproduce the problem is always good as well.
* Submit your patches as [pull requests](https://github.com/Lastique/scope/compare) against **develop** branch. Note that by submitting patches you agree to license your modifications under the [Boost Software License, Version 1.0](https://www.boost.org/LICENSE_1_0.txt).

### License

Distributed under the [Boost Software License, Version 1.0](https://www.boost.org/LICENSE_1_0.txt).
