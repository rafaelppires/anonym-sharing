-- example HTTP POST script which demonstrates setting the
-- HTTP method, body, and adding a header

uuid = require"uuid"

wrk.method = "POST"
wrk.headers["Content-Type"] = "application/json"
counter = 0

request = function(args)
	local path = "/access/user";
    wrk.body = '{"user_id":"' .. uuid() .. '"}'
    counter = counter + 1
	return wrk.format(nil, path)
end

