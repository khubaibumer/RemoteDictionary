# RemoteDictionary

[![CodeFactor](https://www.codefactor.io/repository/github/khubaibumer/remotedictionary/badge)](https://www.codefactor.io/repository/github/khubaibumer/remotedictionary)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/07ee2ee2ec934ddb8ae3491dd40a678e)](https://www.codacy.com/gh/khubaibumer/RemoteDictionary/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=khubaibumer/RemoteDictionary&amp;utm_campaign=Badge_Grade)
[![Softacheck](https://softacheck.com/app/repository/khubaibumer/RemoteDictionary/badge)](https://softacheck.com/app/repository/khubaibumer/RemoteDictionary/issues)
[![Documentation](https://softacheck.com/app/repository/khubaibumer/RemoteDictionary/documentation/badge)](https://softacheck.com/app/docs/khubaibumer/RemoteDictionary/)

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
>> Get Minimum Response Time

> STATS max
>> Get Maximum Response Time

> STATS avg
>> Get Average Response Time

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
    -b, --benchmark        Enables Benchmarking (Disabled by Default)
