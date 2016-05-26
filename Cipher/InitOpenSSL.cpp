#include "InitOpenSSL.h"
#include <openssl\evp.h>
#include <openssl\err.h>
#include <mutex>
#include <sstream>

__LIB_NAME_SPACE_BEGIN__

static int  InitOpenSSL()
{
	OpenSSL_add_all_ciphers();
	OpenSSL_add_all_digests();
	::atexit(EVP_cleanup);
	return 0;
}

static int init_open_ssl = InitOpenSSL();

static void free_err_env()
{
	ERR_free_strings();
	ERR_remove_thread_state(nullptr);
}

static void init_openssl_err_env()
{
	ERR_load_crypto_strings();
	::atexit(free_err_env);
}

static std::once_flag OpensslErrEnvInitFlag;

std::string GetOpensslErrInfo()
{
	std::call_once(OpensslErrEnvInitFlag, init_openssl_err_env);
	const char *file, *data;
	int line, flags;
	unsigned long err = ERR_get_error_line_data(&file, &line, &data, &flags);

	std::ostringstream ss;
	do
	{
		if (0 != err)
		{
			ss << "openssl error occured : " << std::endl
			   <<"errno = " << err << std::endl
			   << "filename = " << file << std::endl
			   << "line number = " << line << std::endl
			   << "data = " << data << std::endl
			   << "reason = " << ERR_reason_error_string(err) << std::endl;
		}
		else
		{
			ss << "no openssl error occured!" << std::endl;
		}
	} while (0 != (err = ERR_get_error_line_data(&file, &line, &data, &flags)));

	return ss.str();
}

__LIB_NAME_SPACE_END__
