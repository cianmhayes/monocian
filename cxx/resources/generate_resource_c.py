import argparse
import os

file_template = """
    #include <stdlib.h>
    const char _resource_{sym}[] = {{
        {body}
    }};
    const size_t _resource_{sym}_len = sizeof(_resource_{sym});
    """

def generate_resource_c(source_path, target_path):
    sym = os.path.split(source_path)[1].replace(".", "_").replace("-", "_")
    os.makedirs(os.path.dirname(target_path), exist_ok=True)
    with open(source_path, "rb") as source_file:
        raw_body = source_file.read().hex(" ")
        body = ", ".join(["0x" + b for b in raw_body.split(" ")])
        with open(target_path, "w") as output_file:
            output_file.write(file_template.format_map({
                "sym": sym,
                "body": body
            }))

if __name__ == "__main__":
    parser = argparse.ArgumentParser(prog='generate_resource_c')
    parser.add_argument("--source", action="store")
    parser.add_argument("--target", action="store")
    args = parser.parse_args()
    generate_resource_c(args.source, args.target)