# DNS Resolver Assignment

This project implements a DNS resolver in Python that supports both **iterative** and **recursive** DNS resolution. The resolver is built using the `dnspython` library and demonstrates how DNS queries are resolved step-by-step.

---

## Features

1. **Iterative DNS Resolution**:
   - Starts querying from the root DNS servers.
   - Follows the DNS hierarchy (Root → TLD → Authoritative) to resolve the domain.
   - Handles timeouts and unreachable servers gracefully.
   - For this assignment(TODO), we have written the `stage` modification `if-else` part in the while loop.

2. **Recursive DNS Resolution**:
   - Uses the system's default DNS resolver to fetch the result recursively.
   - Prints the resolved IP address(es) for the given domain.
   - For this assignment(TODO), we have written the line for getting NS records for the domain using the system dns resolver (`dns.resolver` of `dnspython ` library)

3. **Customizable Root Servers**:
   - Supports a predefined list of root DNS servers for iterative resolution.

4. **Execution Time Measurement**:
   - Measures and prints the time taken for each resolution.

5. **Send query function**
    - Sends UDP query to the server for getting `type A` record for resolving domain.
    - In TODO, we have written the command for sending query via UDP.

6. **Extracting next nameservers function**
    - In this function, we extract nameserver (NS) records from the authority section of the response and then resolve those NS names to IP addresses.
    - In TODO, we use dnsresolver (system dns resolver) of `dnspython` library to get IP addresses from hostnames extracted from the response.


---

## Requirements

- Python 3.x
- `dnspython` library (`pip install dnspython`)

---

## Usage

Run the script from the command line with the following syntax:

```bash
python3 dns_server.py <mode> <domain>