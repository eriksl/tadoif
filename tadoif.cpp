#include "tadoif.h"

#include <dbus-tiny.h>

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>

#include <boost/program_options.hpp>
#include <boost/format.hpp>
#include <boost/json.hpp>
#include <boost/thread.hpp>
#include <boost/chrono.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <string>
#include <vector>
#include <list>
#include <fstream>
#include <sstream>
#include <iostream>

TadoIf::TadoIf()
{
	data.id = 0;
}

TadoIf::~TadoIf()
{
}

void TadoIf::_update()
{
	try
	{
		std::ifstream refresh_token_in_fd;
		std::ofstream refresh_token_out_fd;
		std::string refresh_token;
		std::string access_token;
		curlpp::Cleanup myCleanup;
		curlpp::Easy handle;
		std::list<std::string> curl_headers;
		std::stringstream ss;
		std::string http_data;
		std::string iso_time;
		boost::json::parser json;
		boost::json::object object;
		boost::json::array array;
		unsigned int current;
		unsigned int zones;
		struct tm tm;
		unsigned int offset;

		refresh_token_in_fd.open(refresh_token_file);

		if(!refresh_token_in_fd)
			throw(InternalException("cannot open (read) refresh token file"));

		refresh_token_in_fd >> refresh_token;
		refresh_token_in_fd.close();

		handle.setOpt(curlpp::options::Url(refresh_token_url));
		handle.setOpt(curlpp::options::Timeout(10));
		handle.setOpt(curlpp::options::Post(true));
		handle.setOpt(curlpp::options::PostFields((boost::format("client_id=1bb50063-6b0c-4d11-bd99-387f4a91cc46&grant_type=refresh_token&refresh_token=%s") % refresh_token).str()));

		ss.clear();
		ss.str("");
		ss << handle;
		http_data = ss.str();

		json.write(http_data);
		object = json.release().as_object();

		if(object.contains("error"))
			throw(InternalException("cannot refresh token"));

		if(!object.contains("access_token"))
			throw(InternalException("no access token in reply from tado"));

		if(!object.contains("refresh_token"))
			throw(InternalException("no refresh token in reply from tado"));

		access_token = object["access_token"].as_string();
		refresh_token = object["refresh_token"].as_string();

		//std::cerr << "access token: " << access_token << std::endl;
		//std::cerr << "refresh token: " << refresh_token << std::endl;

		refresh_token_out_fd.open(refresh_token_file);

		if(!refresh_token_out_fd)
			throw(InternalException("cannot open (write) refresh token file"));

		refresh_token_out_fd << refresh_token << std::endl;
		refresh_token_out_fd.close();

		curl_headers.clear();
		curl_headers.insert(curl_headers.begin(), (boost::format("Authorization: Bearer %s") % access_token).str());

		handle.setOpt(curlpp::options::Url(home_url));
		handle.setOpt(curlpp::options::Post(false));
		handle.setOpt(curlpp::options::HttpHeader(curl_headers));

		ss.clear();
		ss.str("");
		ss << handle;
		http_data = ss.str();

		//std::cerr << "output: " << http_data << std::endl;

		json.write(http_data);
		object = json.release().as_object();

		if(object.contains("error"))
			throw(InternalException("cannot get id"));

		if(!object.contains("homeId"))
			throw(InternalException("no homeId field in reply from tado"));

		data.id = object["homeId"].as_int64();

		// std::cerr << "id: " << data.id << std::endl;

		handle.setOpt(curlpp::options::Url((boost::format("%s/%u/%s") % homes_url % data.id % "zones").str()));

		ss.clear();
		ss.str("");
		ss << handle;
		http_data = ss.str();

		// std::cerr << "output: " << http_data << std::endl;

		json.write(http_data);
		array = json.release().as_array();

		zones = array.size();
		data.zones.clear();
		data.zones.resize(zones);

		for(current = 0; current < zones; current++)
		{
			object = array[current].as_object();
			//std::cerr << std::endl;
			//std::cerr << object << std::endl;

			data.zones[current].id = object["id"].as_int64();
			data.zones[current].name = object["name"].as_string();
		}

		for(current = 0; current < zones; current++)
		{
			handle.setOpt(curlpp::options::Url((boost::format("%s/%u/%s/%u/state") % homes_url % data.id % "zones" % data.zones[current].id).str()));

			ss.clear();
			ss.str("");
			ss << handle;
			http_data = ss.str();

			std::cerr << "output: " << http_data << std::endl;

			json.write(http_data);
			object = json.release().as_object();

			if(object.contains("error"))
				throw(InternalException("cannot get data for zone"));

			data.zones[current].active = object["setting"].as_object()["power"].as_string() == "ON" ? true : false;
			data.zones[current].power = object["activityDataPoints"].as_object()["heatingPower"].as_object()["percentage"].as_double() / 100.0;
			data.zones[current].temperature = object["sensorDataPoints"].as_object()["insideTemperature"].as_object()["celsius"].as_double();
			data.zones[current].humidity = object["sensorDataPoints"].as_object()["humidity"].as_object()["percentage"].as_double();
			iso_time = object["sensorDataPoints"].as_object()["insideTemperature"].as_object()["timestamp"].as_string();

			if(iso_time.back() == 'Z')
			{
				offset = 3600;
				iso_time.pop_back();
			}
			else
				offset = 0;

			std::cerr << iso_time << std::endl;
			boost::posix_time::ptime ptime(boost::posix_time::from_iso_extended_string(iso_time));
			tm = boost::posix_time::to_tm(ptime);
			data.zones[current].time = mktime(&tm) + offset;
		}
	}
	catch(const boost::system::system_error &e)
	{
		throw(InternalException(boost::format("json: %s") % e.what()));
	}
	catch(const curlpp::RuntimeError &e)
	{
		throw(InternalException(boost::format("curl runtime: %s") % e.what()));
	}
	catch(const curlpp::LogicError &e)
	{
		throw(InternalException(boost::format("curl logic: %s") % e.what()));
	}
	catch(const InternalException &e)
	{
		throw(Exception(boost::format("error: %s") % e.what()));
	}
	catch(const std::exception &e)
	{
		throw(Exception(boost::format("std::exception: %s") % e.what()));
	}
}

void TadoIf::_fetch(const std::vector<std::string> &argv)
{
	boost::program_options::options_description main_options("usage");
	boost::program_options::positional_options_description positional_options;

	try
	{
		bool proxy;
		bool debug;
		unsigned int zones;

		positional_options.add("host", -1);
		main_options.add_options()
			("proxy,p",		boost::program_options::bool_switch(&proxy)->implicit_value(true),	"run proxy")
			("debug,d",		boost::program_options::bool_switch(&debug)->implicit_value(true),	"show fetched information from proxy")
			("zones,z",		boost::program_options::value(&zones),								"set number of zones to be fetched");

		boost::program_options::variables_map varmap;
		boost::program_options::store(boost::program_options::command_line_parser(argv).options(main_options).positional(positional_options).run(), varmap);
		boost::program_options::notify(varmap);

		//if(proxy)
			//_run_proxy(debug);
		//else
			_update();
	}
	catch(const boost::program_options::required_option &e)
	{
		throw(Exception(boost::format("%s\n%s") % e.what() % main_options));
	}
	catch(const boost::program_options::error &e)
	{
		throw(Exception(boost::format("%s\n%s") % e.what() % main_options));
	}
	catch(const InternalException &e)
	{
		throw(Exception(boost::format("internal error: %s") % e.what()));
	}
	catch(const std::exception &e)
	{
		throw(Exception(boost::format("std::exception: %s") % e.what()));
	}
	catch(const std::string &e)
	{
		throw(Exception(boost::format("std::string exception: %s") % e));
	}
	catch(const char *e)
	{
		throw(Exception(boost::format("unknown charptr exception: %s") % e));
	}
	catch(...)
	{
		throw(Exception(boost::format("unknown exception")));
	}
}

const TadoIf::Data& TadoIf::fetch(const std::vector<std::string> &args)
{
	_fetch(args);

	return(data);
}

const TadoIf::Data& TadoIf::fetch(int argc, const char * const *argv)
{
	int ix;
	std::vector<std::string> args;

	for(ix = 1; ix < argc; ix++)
		args.push_back(std::string(argv[ix]));

	_fetch(args);

	return(data);
}

const TadoIf::Data& TadoIf::fetch(const std::string &args)
{
	std::vector<std::string> args_split;

	args_split = boost::program_options::split_unix(args);

	_fetch(args_split);

	return(data);
}


#if 0
TadoIf::ProxyThread::ProxyThread(TadoIf &tadoif_in, bool debug_in, const std::string &hostname_in) : tado_if(tadoif_in), debug(debug_in), hostname(hostname_in)
{
}

void TadoIf::ProxyThread::operator()()
{
	std::string message_type;
	std::string message_interface;
	std::string message_method;
	std::string error;
	std::string reply;
	std::string time_string;
	std::string service = (boost::format("%s.%s") % dbus_service_id % hostname).str();

	try
	{
		DbusTinyServer dbus_tiny_server(service);

		for(;;)
		{
			try
			{
				dbus_tiny_server.get_message(message_type, message_interface, message_method);

				if(message_type == "method call")
				{
					std::cerr << boost::format("message received, interface: %s, method: %s\n") % message_interface % message_method;

					if(message_interface == "org.freedesktop.DBus.Introspectable")
					{
						if(message_method == "Introspect")
						{
							reply += std::string("") +
										"<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS Object Introspection 1.0//EN\" \"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\">\n" +
										"<node>\n" +
										"	<interface name=\"org.freedesktop.DBus.Introspectable\">\n" +
										"		<method name=\"Introspect\">\n" +
										"			<arg name=\"xml\" type=\"s\" direction=\"out\"/>\n" +
										"		</method>\n" +
										"	</interface>\n" +
										"	<interface name=\"" + service + "\">\n" +
										"		<method name=\"dump\">\n" +
										"			<arg name=\"info\" type=\"s\" direction=\"out\"/>\n" +
										"		</method>\n" +
										"		<method name=\"get_data\">\n" +
										"			<arg name=\"time\" type=\"t\" direction=\"out\"/>\n" +
										"			<arg name=\"type\" type=\"s\" direction=\"out\"/>\n" +
										"			<arg name=\"host_name\" type=\"s\" direction=\"out\"/>\n" +
										"			<arg name=\"host_alias\" type=\"s\" direction=\"out\"/>\n" +
										"			<arg name=\"power\" type=\"d\" direction=\"out\"/>\n" +
										"			<arg name=\"voltage\" type=\"d\" direction=\"out\"/>\n" +
										"			<arg name=\"current\" type=\"d\" direction=\"out\"/>\n" +
										"			<arg name=\"temperature\" type=\"d\" direction=\"out\"/>\n" +
										"		</method>\n" +
										"	</interface>\n" +
										"</node>\n";

							dbus_tiny_server.send_string(reply);

							reply.clear();
						}
						else
							throw(InternalException(dbus_tiny_server.inform_error(std::string("unknown introspection method called"))));
					}
					else
					{
						if((message_interface == dbus_service_id) || (message_interface == ""))
						{
							if(message_method == "dump")
							{
								char timestring[64];
								const struct tm *tm;
								time_t timestamp;

								timestamp = tadoif.data.time;
								tm = localtime(&timestamp);
								strftime(timestring, sizeof(timestring), "%Y-%m-%d %H:%M:%S", tm);

								reply = (boost::format("%-16s %-13s %-19s %5s %8s %7s %11s %-4s\n") %
										"host" % "type" % "alias" % "power" % "voltage" % "current" % "temperature" % "time").str();

								reply += (boost::format("%-16s %-13s %-19s %5.1f %8.1f %7.1f %11.1f %s\n") %
										tadoif.data.host_name %
										tadoif.data.type %
										tadoif.data.host_alias %
										tadoif.data.power %
										tadoif.data.voltage %
										tadoif.data.current %
										tadoif.data.temperature %
										timestring).str();

								dbus_tiny_server.send_string(reply);
							}
							else
							{
								if(message_method == "get_data")
								{
									dbus_tiny_server.send_uint64_x3string_x4double(tadoif.data.time,
											tadoif.data.type, tadoif.data.host_name, tadoif.data.host_alias,
											tadoif.data.power, tadoif.data.voltage, tadoif.data.current, tadoif.data.temperature);
								}
								else
									throw(InternalException(dbus_tiny_server.inform_error(std::string("unknown method called"))));
							}
						}
						else
							throw(InternalException(dbus_tiny_server.inform_error((boost::format("message not for our interface: %s") % message_interface).str())));
					}
				}
				else if(message_type == "signal")
				{
					if(debug)
						std::cerr << boost::format("signal received, interface: %s, method: %s\n") % message_interface % message_method;

					if((message_interface == "org.freedesktop.DBus") && (message_method == "NameAcquired"))
					{
						if(debug)
							std::cerr << "name on dbus acquired\n";
					}
					else
						throw(InternalException(boost::format("message of unknown type: %u") % message_type));
				}
			}
			catch(const InternalException &e)
			{
				std::cerr << boost::format("warning: %s\n") % e.what();
			}

			dbus_tiny_server.reset();
		}
	}
	catch(const DbusTinyException &e)
	{
		std::cerr << "tadoif proxy: fatal: " << e.what() << std::endl;
		exit(1);
	}
	catch(...)
	{
		std::cerr << "tadoif proxy: unknown fatal exception" << std::endl;
		exit(1);
	}
}

void TadoIf::_run_proxy(const std::string &host, bool legacy, bool debug)
{
	proxy_thread_class = new ProxyThread(*this, debug, host);
	boost::thread proxy_thread(*proxy_thread_class);
	proxy_thread.detach();

	for(;;)
	{
		try
		{
			_update();

			if(debug)
			{
				char timestring[64];
				const struct tm *tm;
				time_t timestamp;

				timestamp = data.time;
				tm = localtime(&timestamp);
				strftime(timestring, sizeof(timestring), "%Y-%m-%d %H:%M:%S", tm);

				for(const auto &zone : data.zones)
				{

					std::cout << boost::format("%s %s %s %s %s %s %s %s\n") %
							"zone #", "id", "name", "active", "power", "temperature", "humidity", "time";
					std::cout << boost::format("%d %s %s %s %f %f %f\n") %
							zone_nr,
							zone.id,
							zone.name,
							zone.active ? "active" : "inactive",
							zone.power * 100,
							zone.temperature,
							zone.humidity,
							timestring;
				}
			}

			boost::this_thread::sleep_for(boost::chrono::duration<unsigned int>(30));
		}
		catch(const InternalException &e)
		{
			std::cerr << "warning: " << e.what() << std::endl;
			continue;
		}
	}
}
#endif
