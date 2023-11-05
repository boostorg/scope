# Copyright 2023 Dmitry Arkhipov (grisumbras@yandex.ru)
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# https://www.boost.org/LICENSE_1_0.txt)

vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO Lastique/scope
  REF 6c14f5ccbdc0cb8dfad082f6f3bd5f0d3cdb8cf1
  SHA512 a4f647cb134a91e24a8f159646bb0912866a03047386579184298f6e5dad0b8c2a6df452390bb21954b313c80ddf2cd1d91ec5cdeb8ec4789a1f3f137f21ed5e
  HEAD_REF develop
  PATCHES 1.83_compat.patch
)

file(
  INSTALL "${SOURCE_PATH}/LICENSE"
  DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}"
  RENAME copyright)
file(
  INSTALL "${SOURCE_PATH}/include/"
  DESTINATION "${CURRENT_PACKAGES_DIR}/include"
  PATTERN "${SOURCE_PATH}/include/*")
