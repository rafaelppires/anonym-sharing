-- This script is for creating an envelope

wrk.method = "POST"
wrk.headers["Content-Type"] = "application/json"

init = function(args)
    wrk.body = '{"user_id": "envelope-user-1", "group_id": "envelope-group-' .. args[1] .. '", "bucket_key": "ag7MS7ENVNo3ftj7GPjk+NN6sCpGqZLJbOwvjCCj+uc="}'
end

--request = function()
--	local path = "verifier/envelope"
--	return wrk.format(nil, path)
--end

