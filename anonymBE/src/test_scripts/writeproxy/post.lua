-- example HTTP POST script which demonstrates setting the
-- HTTP method, body, and adding a header

uuid = require"uuid"

wrk.method = "PUT"
--f=io.open("100k.txt")
f=io.open("1M.txt")
wrk.body=f:read("*all")
--wrk.body   = '{\n\t"policy": "session:\\n alias: example.com:CAS.enclaves\\n mrenclaves:\\n - name: CAS\\n operations:\\n - 1f79a193ca4cf61e4015d85580611a3082201102ca4a4c676bf2de3ff4b1e015\\n debug:\\n - 4356a193ca4cf61e4015d85580611a3082201102ca4a4c676bf2de3ff4b1e222\\n - 5356a193ca4cf61e4015d85580611a3082201102ca4a4c676bf2de3ff4b1e223",\n\t"oldpolicy": "' .. uuid() .. '"\n}'
wrk.headers["Content-Type"] = "application/json"
counter = 0

request = function(args)
	local path = "/approve/" .. uuid()
    counter = counter + 1
	return wrk.format(nil, path)
end

