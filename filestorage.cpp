#include "filestorage.hpp"


void filestorage::save_string(t_filestore file_type,
							  const std::string &filename,
							  const std::string &data) {

	if (filename == "") {
		throw std::invalid_argument("Fail to open file for write: empty filename");
	}

	fs::path file_with_path = prepare_file_for_write(file_type, filename);
	fs::ofstream file(file_with_path, std::ios::out | std::ios::binary);

	if (file.is_open()) {
		file << data;
	} else {
		throw std::invalid_argument("Fail to open file for write: " + filename);
	}
	file.close();
}

void filestorage::save_string_mlocked(t_filestore file_type,
									  const std::string &filename,
									  const sodiumpp::locked_string &locked_data) {

	if (filename == "") {
		throw std::invalid_argument("Fail to open file for write: empty filename");
	}

	fs::path file_with_path = prepare_file_for_write(file_type, filename);
	FILE *f_ptr;

	f_ptr = std::fopen(file_with_path.c_str(), "w");
	// magic 1 is the size in bytes of each element to be written
	std::fwrite(locked_data.c_str(), 1, locked_data.size(), f_ptr);

	std::fclose(f_ptr);
}

std::string filestorage::load_string(t_filestore file_type,
									 const std::string &filename) {
	std::string content;

	fs::path file_with_path = get_path_for(file_type);
	file_with_path += filename;
	if (!is_file_ok(file_with_path.native())) {
		throw std::invalid_argument("Fail to open file for read: " + filename);
	} else {
		fs::ifstream ifs(file_with_path);
		content.assign( (std::istreambuf_iterator<char>(ifs) ),
						(std::istreambuf_iterator<char>()  ) );

		ifs.close();
	}
	return content;
}

sodiumpp::locked_string filestorage::load_string_mlocked(t_filestore file_type,
														 const std::string &filename) {
	FILE * f_ptr;
	fs::path file_with_path = get_path_for(file_type);
	file_with_path += filename;

	f_ptr = std::fopen(file_with_path.c_str(), "r");

	if (f_ptr == NULL){
		throw std::invalid_argument("Fail to opening file for read: " + filename);
	}

	std::fseek(f_ptr, 0L, SEEK_END);
	size_t content_size = static_cast<size_t>(ftell(f_ptr));

	std::rewind(f_ptr);

	sodiumpp::locked_string content(content_size);

	size_t byte_read = std::fread(content.buffer_writable(), 1, content_size, f_ptr);
	if (byte_read != content_size) {
		throw std::invalid_argument("Fail to read all content of file: "s + filename
									+ " read: ["s + std::to_string(byte_read)
									+ "] bytes of ["s + std::to_string(content_size) + "]"s);
	}
	std::fclose(f_ptr);

	return content;
}

bool filestorage::is_file_ok(const std::string &filename) {
	fs::path p(filename);

	try {
		if (fs::exists(p)) {    // does p actually exist?
			if (fs::is_regular_file(p)) {       // is p a regular file?
				// std::cout << p << " size is " << file_size(p) << std::endl;  //dbg
			} else if (fs::is_directory(p)) {     // is p a directory?
				std::cout << p << " is a directory" << std::endl;
				return 0;
			} else {
				std::cout << p << " exists, but is neither a regular file nor a directory"
						  << std::endl;
				return 0;
			}
		} else {
			std::cout << p << " does not exist" << std::endl;
			return 0;
		}
	} catch (const fs::filesystem_error& ex) {
		std::cout << ex.what() << std::endl;
		return 0;
	}
	return 1;
}

bool filestorage::remove(const std::string &p) {
	fs::path path_to_remove(p);
	return fs::remove(path_to_remove);
}

fs::path filestorage::prepare_file_for_write(t_filestore file_type,
											 const std::string &filename) {
	fs::path file_with_path;
	try {
		// creating directory tree if necessary
		fs::path full_path = create_path_for(file_type);

		// connect parent path with filename
		file_with_path = full_path;
		file_with_path += filename;
		switch (file_type) {
			case e_filestore_galaxy_ipkeys_pub: {
				file_with_path += ".public";
				//mod=0700 for private key
				break;
			}
			case e_filestore_wallet_galaxy_ipkeys_PRV: {
				file_with_path += ".PRIVATE";
				//mod=0755 for public
				break;
			}
		}
		// std::cout << file_with_path << std::endl; //dbg

		// In code below we want to create an empty file which will help us to open and write down it without any errors
		boost::filesystem::ofstream empty_file;
		empty_file.open(file_with_path);
		empty_file.close();
		if (!is_file_ok(file_with_path.c_str())) {
			std::string err_msg(__func__ + ": fail to create empty file on given path and name"s);
			throw std::invalid_argument(err_msg);
		}
	} catch (fs::filesystem_error & err) {
		std::cout << err.what() << std::endl;
	}
	return file_with_path;
}

fs::path filestorage::create_path_for(t_filestore file_type) {

	fs::path full_path(get_path_for(file_type));
	create_parent_dir(full_path.native());
	// std::cout << user_home << std::endl; //dbg
	// std::cout << full_path << std::endl; //dbg
	return full_path;
}

fs::path filestorage::get_path_for(t_filestore file_type) {

	fs::path user_home(getenv("HOME"));
	fs::path full_path(user_home.c_str());
	switch (file_type) {
		case e_filestore_wallet_galaxy_ipkeys_PRV: {
			full_path += "/.config/antinet/galaxy42/wallet/";
			break;
		}
		case e_filestore_galaxy_ipkeys_pub: {
			full_path += "/.config/antinet/galaxy42/";
			break;
		}
	}
	return full_path;
}

bool filestorage::create_parent_dir(const std::string &filename) {

	fs::path file(filename);
	fs::path parent_path = file.parent_path();

	// if exist
	if (!fs::exists(parent_path)) {
		bool success = fs::create_directories(parent_path);
		if (!success) {
			throw std::invalid_argument("fail to create not existing directory"
										+ std::string(parent_path.c_str()));
		}
		return 1;
	}
	return 0;
}