# tadoif
Successor to Tado::IF for querying Tado devices.

The simple form just queries all zones and displays main data on stdout.

The more interesting form is the proxy, which frequently (every minute now) queries the Tado service and serves this data over the DBus. The proxy can be queried using any standard DBus interface tool or using DBusTinyClient from DBus::Tiny by me (shared library for C++ and XS bindings by SWIG for Perl).
