#pragma once

#include <string>
#include <vector>
#include <exception>
#include <boost/format.hpp>

class TadoIf
{
	public:

		typedef struct
		{
			time_t time;
			unsigned int id;
			std::string name;
			bool active;
			double power;
			double temperature;
			double humidity;
		} Zone;

		typedef struct
		{
			unsigned int id;
			std::vector<Zone> zones;
		} Data;

		class Exception : public std::exception
		{
			public:

				Exception() = delete;
				Exception(const std::string &what);
				Exception(const char *what);
				Exception(const boost::format &what);

				const char *what() const noexcept;

			private:

				const std::string what_string;
		};

		TadoIf();
		TadoIf(const TadoIf &) = delete;
		~TadoIf();

		const Data &fetch(const std::vector<std::string> &);
		const Data &fetch(int argc, const char * const *argv);
		const Data &fetch(const std::string &);

	private:

		static constexpr const char *dbus_service_id = "name.slagter.erik.tadoif";
		static constexpr const char *refresh_token_file = "/var/local/tado/refresh_token";
		static constexpr const char *refresh_token_url = "https://login.tado.com/oauth2/token";
		static constexpr const char *home_url = "https://my.tado.com/api/v1/me";
		static constexpr const char *homes_url = "https://my.tado.com/api/v2/homes";

		class InternalException : public std::exception
		{
			public:

				InternalException() = delete;
				InternalException(const std::string &what);
				InternalException(const char *what);
				InternalException(const boost::format &what);

				const char *what() const noexcept;

			private:

				const std::string what_string;
		};

		class ProxyThread
		{
			public:

				ProxyThread(TadoIf &, bool debug);
				__attribute__((noreturn)) void operator ()();

			private:

				TadoIf &tadoif;
				bool debug;
		};

		void _update(bool debug);
		void _fetch(const std::vector<std::string> &);

		__attribute__((noreturn)) void _run_proxy(bool debug);

		Data data;
		ProxyThread *proxy_thread_class;
};
