# RemoteDictionary

## Request Syntax

> GET key
>> Get Value for specified key

> SET key value
>> Set Key and Value (adds to dictionary)

> UPDATE key value
>> Update Value for existing Key

> STATS dictionary
>> Get Stats for GET request

> STATS min
>> Get Per Worker Thread Minimum Response Time

> STATS max
>> Get Per Worker Thread Maximum Response Time

> STATS avg
>> Get Per Worker Thread Average Response Time

> STATS summary
>> Get OverAll Response Time

## Server Options

    -i, --ip               Server IP Address
    -p, --port             Port Number
    -j, --threads          Number of Worker Threads (Default is 10)
    -f, --enable_filter    Enables Bloom Filter

## Client Options

    -i, --ip               Server IP Address
    -p, --port             Port Number
    -b, --benchmark        Enables Benchmarking (Enabled by Default)
