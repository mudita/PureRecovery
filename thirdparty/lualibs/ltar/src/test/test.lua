local tar = require("ltar")
local lfs = require("lfs")

function delete_dir(dir)
    for file in lfs.dir(dir) do
        local file_path = dir .. "/" .. file
        if file ~= "." and file ~= ".." then
            if lfs.attributes(file_path, "mode") == "file" then
                os.remove(file_path)
            elseif lfs.attributes(file_path, "mode") == "directory" then
                delete_dir(file_path)
            end
        end
    end
    lfs.rmdir(dir)
end

function capture(cmd, raw)
    local f = assert(io.popen(cmd, 'r'))
    local s = assert(f:read('*a'))
    f:close()
    return s
end

--- Test case: Unpack sample.tar and then pack the content. Compare it with the original sample and its content.

os.execute("mkdir sample")
os.execute("tar xf sample.tar -C sample")

tar.unpack("sample.tar", ".")
tar.create_from_path("lua", "test.tar")

local sample_file_size = lfs.attributes("sample.tar").size
local test_file_size = lfs.attributes("test.tar").size
assert(sample_file_size == test_file_size, string.format("size: %u, should be: %u", test_file_size, sample_file_size))

--Compare the contents of original directory and the generated one after up-packing. They should be identical.
assert(capture("diff -qrN sample/lua lua") == '', "Directories content is not identical")

delete_dir("sample")
delete_dir("lua")

--- Test case: Append directory and compare it with the original sample and its content.

os.execute("tar xf sample_to_append.tar")
tar.append("lualibs", "test.tar")

local sample_appended_size = lfs.attributes("sample_appended.tar").size
local test_appended_size = lfs.attributes("test.tar").size
assert(sample_appended_size == test_appended_size, string.format("size: %u, should be: %u", test_appended_size, sample_appended_size))

tar.unpack("test.tar", "appended")
os.execute("mkdir sample_appended")
os.execute("tar xf sample_appended.tar -C sample_appended")
--Compare the contents of original directory and the generated one after up-packing. They should be identical.
assert(capture("diff -qrN appended sample_appended") == '', "Directories content is not identical")

os.remove("test.tar")
delete_dir("lualibs")
delete_dir("appended")
delete_dir("sample_appended")


--
--local handle = tar.create("create.tar")
--handle:add_directory("matipati")
--handle:add_directory("matipati222")
--handle:add_file("build.ninja")
--handle:close()

--print(tar.find("create.tar","build.ninja"))
