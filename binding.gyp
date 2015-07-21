{
    "targets": [
        {
            "type": "loadable_module",
            "target_name": "zipper",
            "variables": {
                "libzip%": "0.10"
            },
            "sources": [
                "src/_zipper.cc",
                "src/zipper.cpp"
            ],
            "conditions": [
                [ 'OS=="mac"', {
                    "include_dirs": [
                        "<!(node -e \"require('nan')\")",
                        "deps/libzip-<(libzip)/lib/"
                    ],
                    "libraries": [
                        "-lz",
                        "-L../deps/libzip-<(libzip)/lib/.libs/",
                        "-lzip"
                    ],
                } ],
                [ 'OS=="linux"', {
                    "include_dirs": [
                        "<!(node -e \"require('nan')\")"
                    ],
                    "libraries": [
                        "-lz",
                        "-lzip"
                    ],
                } ]
            ]
        }
    ]
}
