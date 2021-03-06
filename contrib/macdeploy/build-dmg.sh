#!/usr/bin/env bash

set -o errexit
set -o nounset

readonly SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
readonly PACKAGE_NAME="Galaxy42_Installer"
readonly PKG_PATH="$SCRIPT_DIR/$PACKAGE_NAME.pkg"
readonly DMG_PATH="$SCRIPT_DIR/$PACKAGE_NAME.dmg"

readonly GALAXY42_VERSION="$(git describe)"


is_dir() {
	local dir="$1"
	[[ -d "$dir" ]]
}

is_file() {
	local file="$1"
	[[ -f "$file" ]]
}

clean_dirs() {
	echo "cleaning dirs"
	dir_list=("$@")
	for dir in "${dir_list[@]}"; do
		is_dir "$dir" \
			&& rm -rf "$dir"
	done
}

clean_files() {
	echo "cleaning files"
	file_list=("$@")
	for file in "${file_list[@]}"; do
		is_file "$file" \
			&& rm "$file"
	done
}

# parse file path to get filename, where '/' is delimiter : /usr/bin/gcc -> gcc
# get_last_column returning one variable, an requires one function argument
# example of use:
#	last=''
#	get_last_column last "/usr/bin/gcc"
#
get_last_column () {
	return_value="$1"
	fun_argument="$2"
	eval "$return_value"="$( echo "$fun_argument" \
		| rev \
		| cut -d/ -f1 \
		| rev )"
}

change_tun_dylib_loadpath () {
	local tun_path="$SCRIPT_DIR/tunserver.app/tunserver.elf"

	# dynamic libs that are required by galaxy42
	local dylib_list=( \
		"/usr/local/opt/boost/lib/libboost_locale-mt.dylib" \
		"/usr/local/opt/boost/lib/libboost_system-mt.dylib" \
		"/usr/local/opt/boost/lib/libboost_filesystem-mt.dylib" \
		"/usr/local/opt/boost/lib/libboost_program_options-mt.dylib" \
		"/usr/local/opt/libsodium/lib/libsodium.18.dylib" )

	# check that required files exist
	! is_file "$tun_path" \
		&& echo "$tun_path not exist, something went wrong, exiting ..." && exit 1

	for dylib in "${dylib_list[@]}"; do
		! is_file "$dylib" \
			&& echo "missing library: $dylib. This library is required on your machine to create pkg, exiting ..." && exit 1
	done

	for dylib in "${dylib_list[@]}"; do
		last_column='overwrite_me'
		get_last_column last_column "$dylib"
		install_name_tool -change "$dylib" "/Applications/Galaxy42.app/$last_column" "$tun_path"
	done
}

create_tun_component () {
	echo "creating tun.pkg component"
	local tun_identifier="tunserver"
	local tun_componenet_app="Tunserver.app"
	local tun_plist="tunserver.plist"
	local tun_bin="tunserver.elf"
	local tun_pkg="tunserver.pkg"


	pushd "$SCRIPT_DIR"
		clean_dirs "$tun_componenet_app"

		mkdir "$tun_componenet_app"

		cp -n "../../$tun_bin" "$tun_componenet_app"
		change_tun_dylib_loadpath

		pkgbuild --analyze --root "$tun_componenet_app" "$tun_plist"
		pkgbuild --identifier "$tun_identifier" \
			--root "$tun_componenet_app" \
			--component-plist "$tun_plist" \
			--install-location "/Applications/Galaxy42.app" \
			"$tun_pkg"

		clean_dirs "$tun_componenet_app"
		clean_files "$tun_plist"
	popd
}

create_boost_component () {
	echo "creating boost.pkg component"
	local boost_identifier="boost_libs"
	local boost_componenet_app="Boost.app"
	local boost_plist="boost.plist"
	local boost_pkg="boost.pkg"

	pushd "$SCRIPT_DIR"
		clean_dirs "$boost_componenet_app"

		mkdir "$boost_componenet_app"
		cp -n "/usr/local/opt/boost/lib/libboost_locale-mt.dylib" "$boost_componenet_app"
		cp -n "/usr/local/opt/boost/lib/libboost_system-mt.dylib" "$boost_componenet_app"
		cp -n "/usr/local/opt/boost/lib/libboost_filesystem-mt.dylib" "$boost_componenet_app"
		cp -n "/usr/local/opt/boost/lib/libboost_program_options-mt.dylib" "$boost_componenet_app"

		pkgbuild --analyze --root "$boost_componenet_app/" "$boost_plist"
		pkgbuild --identifier "$boost_identifier" \
			--root "$boost_componenet_app" \
			--install-location "/Applications/Galaxy42.app" \
			"$boost_pkg"

		clean_dirs "$boost_componenet_app"
		clean_files "$boost_plist"
	popd
}

create_sodium_component () {
	echo "creating sodium.pkg component"
	local sodium_identifier="libsodium"
	local sodium_componenet_app="Sodium.app"
	local sodium_plist="sodium.plist"
	local sodium_pkg="sodium.pkg"

	pushd "$SCRIPT_DIR"
		clean_dirs "$sodium_componenet_app"

		mkdir "$sodium_componenet_app"
		cp -n "/usr/local/opt/libsodium/lib/libsodium.18.dylib" "$sodium_componenet_app"

		pkgbuild --analyze --root "$sodium_componenet_app" "$sodium_plist"
		pkgbuild --identifier "$sodium_identifier" \
			--root "$sodium_componenet_app" \
			--install-location "/Applications/Galaxy42.app" \
			"$sodium_pkg"

		clean_dirs "$sodium_componenet_app"
		clean_files "$sodium_plist"
	popd
}

create_galaxy_pkg() {
	echo "creating galaxy42.pkg"
	create_tun_component
	create_boost_component
	create_sodium_component

	pushd "$SCRIPT_DIR"
		productbuild --synthesize \
				--package tunserver.pkg \
				--package boost.pkg \
				--package sodium.pkg \
				Distribution.xml

		productbuild --distribution ./Distribution.xml \
				--package-path . \
				"$PKG_PATH"

		clean_files "Distribution.xml" "tunserver.pkg" "boost.pkg" "sodium.pkg"
	popd
}

create_galaxy_dmg() {
	local tmp_name="gal_tmp.dmg"
	local vol_name="galaxy42_volume"
	local vol_size="10000k"

	pushd "$SCRIPT_DIR"
		clean_files "$tmp_name" "${DMG_PATH}"

		hdiutil create -srcfolder "${PKG_PATH}" \
			-volname "${vol_name}" \
			-fs HFS+ \
			-fsargs "-c c=64,a=16,e=16" \
			-format UDRW \
			-size "${vol_size}" \
			"${tmp_name}"

		device="$(hdiutil attach -readwrite -noverify -noautoopen "${tmp_name}" | egrep '^/dev/' | sed 1q | awk '{print $1}' )"

		echo "sudo is necessary in this step to setting permissions properly:"
		set -x
		sudo chmod -Rf go-w "/Volumes/${vol_name}"
		set +x

		sync
		sync

		hdiutil detach "${device}"
		hdiutil convert "${tmp_name}" -format UDZO -imagekey zlib-level=9 -o "${DMG_PATH}"

		clean_files "${tmp_name}" "${PKG_PATH}"
	popd
}

main() {
	echo "starting main"
	create_galaxy_pkg
	create_galaxy_dmg
}
main
