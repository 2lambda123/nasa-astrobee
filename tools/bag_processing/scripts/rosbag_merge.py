#!/usr/bin/env python3
# Copyright (c) 2017, United States Government, as represented by the
# Administrator of the National Aeronautics and Space Administration.
#
# All rights reserved.
#
# The Astrobee platform is licensed under the Apache License, Version 2.0
# (the "License"); you may not use this file except in compliance with the
# License. You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations
# under the License.
"""
Merges bagfiles with given prefix in the current working directory. 
"""


import argparse
import os
import re
import string
import sys

import rosbag


# https://stackoverflow.com/a/4836734
def natural_sort(l):
    convert = lambda text: int(text) if text.isdigit() else text.lower()
    alphanum_key = lambda key: [convert(c) for c in re.split("([0-9]+)", key)]
    return sorted(l, key=alphanum_key)


def merge_bag(input_bag_prefix, input_bag_suffix, merged_bag, only_loc_topics=False):
    # Find bagfiles with bag prefix in current directory, fail if none found
    bag_names = [
        bag
        for bag in os.listdir(".")
        if os.path.isfile(bag)
        and bag.startswith(input_bag_prefix)
        and bag.endswith(input_bag_suffix)
    ]
    if len(bag_names) == 0:
        print("No bag files found")
        sys.exit()
    elif len(bag_names) == 1:
        print("Only one, nothing to merge")
        return
    else:
        print(("Found " + str(len(bag_names)) + " bag files."))

    merged_bag_name = ""
    if not merged_bag:
        merged_bag_name = input_bag_prefix + ".merged.bag"

    sorted_bag_names = natural_sort(bag_names)

    topics = None
    if only_loc_topics:
        topics = [
            "/hw/imu",
            "/loc/of/features",
            "/loc/ml/features",
            "/loc/ar/features",
            "/mgt/img_sampler/nav_cam/image_record",
            "/graph_loc/state",
            "/gnc/ekf",
            "/sparse_mapping/pose",
            "/mob/flight_mode",
            "/beh/inspection/feedback",
            "/beh/inspection/goal",
            "/beh/inspection/result",
        ]

    with rosbag.Bag(merged_bag_name, "w") as merged_bag:
        for sorted_bag_name in sorted_bag_names:
            with rosbag.Bag(sorted_bag_name, "r") as sorted_bag:
                for topic, msg, t in sorted_bag.read_messages(topics):
                    merged_bag.write(topic, msg, t)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.ArgumentDefaultsHelpFormatter
    )
    parser.add_argument(
        "input_bag_prefix",
        nargs="*",
        help="Prefix for bagfiles to merge. Bags should all be in the current working directory.",
    )
    parser.add_argument(
        "--input-bag-suffix",
        default=".bag",
        help="Filter suffix for bagfiles to merge. Bags should all be in the current working directory.",
    )
    parser.add_argument(
        "--merged-bag",
        default="",
        help="Output merged bag. By default this is merged_prefix.bag where prefix is the provided bag prefix.",
    )
    parser.add_argument(
        "-d",
        "--directory",
        default=".",
        help="Directory to where to find the bags",
    )
    parser.add_argument(
        "--only-loc-topics",
        dest="only_loc_topics",
        action="store_true",
        help="Only save loc topics to output merged bag.",
    )
    args = parser.parse_args()
    os.chdir(args.directory)

    bag_names = args.input_bag_prefix
    if not bag_names:
        # Find bagfiles with bag prefix in current directory, fail if none found
        bag_names = [
            bag[: -len(args.input_bag_suffix)].rstrip(string.digits)
            for bag in os.listdir(".")
            if os.path.isfile(bag) and bag.endswith(args.input_bag_suffix)
        ]
        # Remove duplicates
        bag_names = sorted(set(bag_names))
        if len(bag_names) == 0:
            print("No bag files found")
            sys.exit()
        else:
            print(("Found " + str(len(bag_names)) + " bag file prefixes."))

    print(bag_names)
    for bag_name in bag_names:
        print(bag_name)
        merge_bag(
            bag_name, args.input_bag_suffix, args.merged_bag, args.only_loc_topics
        )
