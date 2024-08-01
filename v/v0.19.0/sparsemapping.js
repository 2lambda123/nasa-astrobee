var sparsemapping =
[
    [ "Creation of sparse maps for robot localization", "sparsemapping.html#autotoc_md439", [
      [ "What is a map", "sparsemapping.html#autotoc_md440", null ],
      [ "Map files", "sparsemapping.html#autotoc_md441", null ],
      [ "ROS node", "sparsemapping.html#autotoc_md442", [
        [ "Residuals", "optimizationcommon.html#autotoc_md344", null ],
        [ "SE3 Local Parameterization", "optimizationcommon.html#autotoc_md345", null ],
        [ "Utilities", "optimizationcommon.html#autotoc_md346", null ],
        [ "PointToPlaneICP", "pointcloudcommon.html#autotoc_md352", null ],
        [ "PointCloudWithKnownCorrespondencesAligner", "pointcloudcommon.html#autotoc_md353", null ],
        [ "Utilities", "pointcloudcommon.html#autotoc_md354", null ],
        [ "Inputs", "sparsemapping.html#autotoc_md443", null ],
        [ "Outputs", "sparsemapping.html#autotoc_md444", null ]
      ] ],
      [ "The environment", "sparsemapping.html#autotoc_md445", null ],
      [ "Tools and procedures", "sparsemapping.html#autotoc_md446", [
        [ "Record a bag", "sparsemapping.html#autotoc_md447", null ],
        [ "Filter the bag", "sparsemapping.html#autotoc_md448", null ],
        [ "Copy the bag from the robot", "sparsemapping.html#autotoc_md449", null ],
        [ "Merging bags", "sparsemapping.html#autotoc_md450", null ],
        [ "Extracting images", "sparsemapping.html#autotoc_md451", null ],
        [ "Building a map", "sparsemapping.html#autotoc_md452", null ],
        [ "Visualization", "sparsemapping.html#autotoc_md453", null ],
        [ "Localize a single frame", "sparsemapping.html#autotoc_md454", null ],
        [ "Testing localization using two maps", "sparsemapping.html#autotoc_md455", null ],
        [ "Testing localization using a bag", "sparsemapping.html#autotoc_md456", null ],
        [ "Extract sub-maps", "sparsemapping.html#autotoc_md457", null ],
        [ "Merge maps", "sparsemapping.html#autotoc_md458", [
          [ "General usage", "sparsemapping.html#autotoc_md459", null ],
          [ "Registration and bundle adjustment", "sparsemapping.html#autotoc_md460", null ],
          [ "Keeping first map fixed", "sparsemapping.html#autotoc_md461", null ]
        ] ],
        [ "How to build a map efficiently", "sparsemapping.html#autotoc_md462", null ],
        [ "Map strategy for the space station", "sparsemapping.html#autotoc_md463", null ],
        [ "Growing a map when more images are acquired", "sparsemapping.html#autotoc_md464", null ],
        [ "Reducing the number of images in a map", "sparsemapping.html#autotoc_md465", null ]
      ] ]
    ] ],
    [ "Build map from multiple bags", "build_map_from_multiple_bags.html", null ],
    [ "Map building", "map_building.html", [
      [ "Detailed explanation", "map_building.html#autotoc_md378", [
        [ "Steps", "build_map_from_multiple_bags.html#autotoc_md419", [
          [ "1. Convert bags from bayer to rgb if necessary", "build_map_from_multiple_bags.html#autotoc_md420", null ],
          [ "2. Splice bags using splice tool", "build_map_from_multiple_bags.html#autotoc_md421", null ],
          [ "3. Build individual SURF maps for each spliced bag", "build_map_from_multiple_bags.html#autotoc_md422", null ],
          [ "4. Merge SURF maps", "build_map_from_multiple_bags.html#autotoc_md423", null ],
          [ "5. Register SURF map using provided world points", "build_map_from_multiple_bags.html#autotoc_md424", null ],
          [ "6. Build BRISK map for localization", "build_map_from_multiple_bags.html#autotoc_md425", null ],
          [ "7. Verify BRISK map using localization", "build_map_from_multiple_bags.html#autotoc_md426", null ]
        ] ],
        [ "Summary", "map_building.html#autotoc_md377", null ],
        [ "Setup the environment", "map_building.html#autotoc_md379", null ],
        [ "Partition the files into movement sequences and reduce the number of images to improve bundle-adjustment accuracy", "map_building.html#autotoc_md380", null ],
        [ "Building a map", "map_building.html#autotoc_md381", [
          [ "Map building pipeline", "map_building.html#autotoc_md382", [
            [ "Detect interest points", "map_building.html#autotoc_md383", null ],
            [ "Match images", "map_building.html#autotoc_md384", null ],
            [ "Build tracks", "map_building.html#autotoc_md385", null ],
            [ "Incremental bundle adjustment", "map_building.html#autotoc_md386", null ],
            [ "Bundle adjustment", "map_building.html#autotoc_md387", null ],
            [ "Map rebuilding", "map_building.html#autotoc_md388", null ],
            [ "Vocabulary database", "map_building.html#autotoc_md389", null ]
          ] ],
          [ "Building a SURF map only", "map_building.html#autotoc_md390", null ],
          [ "Additional options", "map_building.html#autotoc_md391", null ]
        ] ],
        [ "Map registration", "map_building.html#autotoc_md392", [
          [ "Registration in the granite lab", "map_building.html#autotoc_md393", null ],
          [ "Registration on the ISS", "map_building.html#autotoc_md394", null ],
          [ "Registration in the MGTF", "map_building.html#autotoc_md395", null ]
        ] ],
        [ "Map verification", "map_building.html#autotoc_md396", null ],
        [ "Sparse map performance and quality evaluation on the robot", "map_building.html#autotoc_md397", [
          [ "Stage the new map", "map_building.html#autotoc_md398", [
            [ "Copy the new map on the robot MLP (preferably in /data):", "map_building.html#autotoc_md399", null ],
            [ "On the MLP, move the current map aside:", "map_building.html#autotoc_md400", null ]
          ] ],
          [ "Stage the bag with images:", "map_building.html#autotoc_md402", null ],
          [ "Stage the feature counter utility (should be added to the install at one point):", "map_building.html#autotoc_md403", null ],
          [ "Launch the localization node on LLP", "map_building.html#autotoc_md404", null ],
          [ "Play the bags (on MLP)", "map_building.html#autotoc_md405", null ],
          [ "Enable localization and the mapped landmark production (on MLP)", "map_building.html#autotoc_md406", null ],
          [ "Examine the performance and features on MLP", "map_building.html#autotoc_md407", [
            [ "Look at the load with htop", "map_building.html#autotoc_md408", null ],
            [ "Watch the frequency of feature production", "map_building.html#autotoc_md409", null ],
            [ "Watch the number of features being produced:", "map_building.html#autotoc_md410", null ]
          ] ]
        ] ],
        [ "Verify localization against a sparse map on a local machine", "map_building.html#autotoc_md411", [
          [ "Preparation", "map_building.html#autotoc_md412", null ],
          [ "Start localization", "map_building.html#autotoc_md413", null ],
          [ "Play the bag", "map_building.html#autotoc_md414", null ],
          [ "Enable localization", "map_building.html#autotoc_md415", null ],
          [ "Alternative approach", "map_building.html#autotoc_md416", null ],
          [ "Examining the results", "map_building.html#autotoc_md417", null ]
        ] ],
        [ "Evaluating the map without running the localization node", "map_building.html#autotoc_md418", null ]
      ] ]
    ] ],
    [ "Total Station", "total_station.html", [
      [ "Doing a Survey with the Total Station", "total_station.html#autotoc_md474", null ]
    ] ],
    [ "Granite Lab Registration", "granite_lab_registration.html", [
      [ "Locations of the control points in the granite lab used for registration", "granite_lab_registration.html#autotoc_md428", [
        [ "Point 1", "granite_lab_registration.html#autotoc_md429", null ],
        [ "Point 2", "granite_lab_registration.html#autotoc_md430", null ],
        [ "Point 3", "granite_lab_registration.html#autotoc_md431", null ],
        [ "Point 4", "granite_lab_registration.html#autotoc_md432", null ]
      ] ]
    ] ],
    [ "Faro", "faro.html", null ],
    [ "Building a map with Theia", "theia_map.html", [
      [ "Install Theia's prerequisites", "theia_map.html#autotoc_md466", null ],
      [ "Fetch and build Theia", "theia_map.html#autotoc_md467", null ],
      [ "Hide the conda environment", "theia_map.html#autotoc_md468", null ],
      [ "Set up the environment for Theia and Astrobee", "theia_map.html#autotoc_md469", null ],
      [ "Prepare the data", "theia_map.html#autotoc_md470", null ],
      [ "Run the Astrobee wrapper around the Theia tools", "theia_map.html#autotoc_md471", null ],
      [ "Command line options", "theia_map.html#autotoc_md472", null ],
      [ "Next steps", "theia_map.html#autotoc_md473", null ]
    ] ],
    [ "Importing a map in .nvm format", "import_map.html", [
      [ "Overview", "import_map.html#autotoc_md433", null ],
      [ "Import an nvm map made with distorted images", "import_map.html#autotoc_md434", null ],
      [ "Import an nvm map made with undistorted images", "import_map.html#autotoc_md435", [
        [ "Keep, after importing, the undistorted images and camera model", "import_map.html#autotoc_md436", null ],
        [ "Replace with distorted data", "import_map.html#autotoc_md437", null ]
      ] ],
      [ "Rebuilding an imported map", "import_map.html#autotoc_md438", null ]
    ] ],
    [ "Exporting a map to the .nvm format", "export_map.html", [
      [ "Overview", "export_map.html#autotoc_md427", null ]
    ] ],
    [ "Merge bags", "merge_bags.html", null ]
];