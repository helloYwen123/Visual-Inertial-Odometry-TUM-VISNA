
## WS2020/21

TODO code:
 - new warnings with tbb and cereal: https://gitlab.vision.in.tum.de/visnav_ss20/s0071/visnav/-/merge_requests/6






## SS2020




### session 2

#### discuss

 - recording
 - introduce ourselves (with David)
 - quickly go over exercise sheet
 - can ask in chat, or with microphone
 - feel free to ask simple questions during the open part
 - if you haven't finished ex0 test merge request, talk to us later
 - after initial Q/A, no fixed content, so feel free to leave (only if you feel comfortable)


### session 3

 - announcements:
     - recording
     - agenda
     - code updates: pull from master
     - how will teams be formed; introduction round?
 - Q&A lecture
     - ...
 - Q&A exercise sheets
     - clang format
     - Notation for transformations T_w_i, T_c
     - local variables, references, ceres parameters
     - WIP merge requests, no need to close and re-open
     - timeouts for CI jobs --> manually retry
     - (t_ns naming; timestamp vs index)
 - go over solution sheet 1 ? discuss handing out solutions ?
     - cmake: link_libraries, template instantiation note limit
     - Taylor expansions: wolfram alpha
     - Taylor expansions again: group terms appropriately, 
       e.g. don't expand sin(theta) if it's in the denominator, but theta / sin(theta)
     - precision issue: float
     - what is a good epsilon? numeric_limits::epsilon


### session 4

- discuss GUI: lock frames, different matching methods
- discuss ex2:
  - camera models newton, abort criteria, num iterations
  - taylor expansion
  - invalid points and intrinsics --> ignored
  - adding parameter blocks before --> better error checking on dimensions



### session 5

- intro round
- start recording
- announcements:
  - dates presentation, report ok?
  - next week no session --> holiday
    - q&a gitlab
    - discussion solutions: video
- Q&A projects, sheets 4&5
- Discuss solution ex3
  - order of md.matches, md.inliers --> checked by tests, but strictly speaking shouldn't matter
  - no need to set rotation and translation before optimize-nonlinear; but only for central-relative-pose-sac-problem




### TODO towards end

- feedback / survey:
  - technical setup; OS; IDE;
  - format lecture / tutorsession





### feedback/todo lectures

 - lecture 1/2:
   - virtual laser pointer

 - solutions




### todo code

 - new warnings with tbb and cereal: https://gitlab.vision.in.tum.de/visnav_ss20/s0071/visnav/-/merge_requests/6
 - #(no: only for central relative pose sac problem) no need to set adapter pose after ransac: update sheets, comments, solution








## WS2019 and earlier
# discuss

Session 3:

- exp/log SE3 -> Jacobian for rot = 0, trans != 0
- inverse Jacobian --> closed form exists

Session 2:

- typo in log SO(3): theta / (2 sin(theta)) (R - R^T)
  theta = arccos ( (tr(T) - 1)/2 )





# info students:



week 5:

- triangulate vs triangulate2

- robust loss
  - outlier --> no inpact from "inaccurate point detection"

- outlier types

- error checking for invalid projection points in BA

- suggestions for improvements:
   - no brute force matching
   - parallel BA --> already
   - keyframes --> already
   - add multiple cameras at a time --> already
   - make BA faster
   - not run BA after every new camera --> some local BA is needed


- evaluation --> do now





week 4:

- don't get too attached to description in exercise sheet; think about
  what we are doing, and how you would solve it on your own; then look
  at the code as a guidance. --> code / exercise is not best or only
  soltuion, just some choice. Try to understand why we are doing
  things the way we are doing them. Some of the text questions aim at
  that.

- order left-right for epipolar constraint
- constraint with 2d vs 3d coordinates --> works w/o depth

- refine RANSAC result: --> general pattern;
- use optimize_nonlinear (reuse adapter), or call optimizeModelParameters




week 3:
- debugging: make sure you can run visual debugger
- sheet 2 dicussion:
  - first add parameter blocks, then residuals
  - parameter blocks, local variables, references
  - ceres::sin, vs std::sin, vs sin
  - not using huber loss
  - Newton abort criteria; fixed number of iterations; check step; (don't check derivative)






- attendance sheet
TODO: machines: get a list of who is working on which machine? (to disable gitlab runner, maybe manual reboot before session)


week 2:
* discuss debug vs relwithdebinfo for debugging
* discuss about eigen pitfalls and useful things: use of `auto`, block operations
* std::sin vs cmath, usage of pow

* references barfoot, ethaneade (closed-form log, inverse J)
* discuss impl. so3 log --> eigenvalue vs direct formula
* discuss point about Taylor expansions




discuss / document:
    * update repos --> pull from upstream (cmake/qtcreator/eigen issue; faq; ex2 tests)
    * sheet2:
      * second unit test to uncomment
      * extrinsic calibration stereo
      * body frame; c0-to-imu
    * lab PCs slowness --> other people working from remote; let us know
    * don't switch off lab machines when leaving, just log off
    * use a separate branch for every exercise; each branch builds on the previous exercise
    * when opening MR: use proper name, indicate when it's ready (remove "WIP")
    * FAQ:
      * info remote access
      * Info about QtCreator and compile "current executable" / `make calibration`











# TODO


* replace submodules with subtree


* sheets general:
 - split into mandatory part, and more open additional voluntary extension / testing / tuning ...



* sheet 5:
- integration test ensuring that everything works --> maybe load GT and check final ATE; same for SFM?


* sheet 4:
- update unit tests to cover more cases (see in-code TODO)


* sheet 3:



* Sheet 2:
- Move json comparison inside c++ unit test (load calib + compare param vector).
- Fix compare-json unit test to be less sensitive to actual implementation.


* project ideas:
  - motion averaging global sfm pipeline


* ask quririn to install all prerequisites on all lab machines + qtcreator-4.8 + htop + aptitude



* protected branches, default branch


* "wiki" setup page
  * 16.04: install cmake (from pip)
  * port mac hints, submission hints, faq
  * note on creating issues in visnav_ss19 main repo; watch this repo and own
  * note in FAQ on frames and and transformations + typo in comment --> Pablo's email
  * include in FAQ note on pulling in updates from `master` or `upstream`.
  * improve setup instructions for macos (with prominent note that the recommended way is to go with the Ubuntu machines)
  * Info about QtCreator and compile "current executable"
  * update screenshots


discuss / document:
    * lab PCs slowness --> let us know
    * don't switch off lab machines when leaving, just log off
    * rename account to your actual name
    * make sure to add email addresses to gitlab profile
    * use a separate branch for every exercise; each branch builds on the previous exercise
    * when opening MR: use proper name, indicate when it's ready (remove "WIP")
    * encourage use of Issues
    * Info about QtCreator and compile "current executable"




## for lecture / practical


## discuss


## organizational


## documentation


## technical


## Next iteration of course

 * split into some more cpp, to reduce compilation times --> check refactoring Johannes Stubenrauch
 * Update course description (website), for example regarding
   modalities of attendnace, aim and contents of the course.
 * Find a design for camera model that allows to instantiate once for
   each residual, not recreating object for every residual
   computation; also find a design that allows to make the vector of
   parameters not constant size 8, but correct size according to the
   actual camera model.
 * In camera projection, use as much (Scalar, Scalar) or (Scalar, T)
   operations as possible, to make Jet types as fast as possible.
 * Check opengv with march native on Vlad's machine
 * for ceres Jet types, discuss "Scalar(1)" vs "1.0" --> update our implementation
 * checkout work on branch camera-model-testing --> avoid allocation (to create camera model objects) in functors and allow different size parameter blocks for intrinsics
 * CMake: check for submodules and give meaningfull error if not present or not built
 * default values for --dataset-path
 * sheet / lecture 4: more references for SfM
 * carefully check image bounds and conversion from continuous to discrete
   coordinates (range shoudl be [-0.5 - width-0.5])
 * consider adding a "Werror Wall unit test" to ci to enforce warning-free builds --> add "UNUSED(...)" to the code templates to make it warning free
 * sheet 3: make it more explicit in the description / code comments that you need to use refinement of the pose with all inliers. Also explain this in the lecture.
 * checkout the repository Vlad found which implements automatic creation of student repos etc... (https://git.uwaterloo.ca/cscf/gitlab-assignments)
 * unify code style (camel case vs underscore, ...) --> do manually or check tools like clang-tidy
 * unit tests: use ensure as much as possible to have failing test show all failures, not just the first.
 * map serialization format: also save intrinsics with the map

 * things to explain at the start (or maybe even write down):
    * when opening MR: use proper name, indicate when it's ready (remove "WIP")
    * encourage use of Issues

 * Are we providing "musterloesung"; how / where? Code; Questions on sheets;
 * Use stereo constraints for residuals
 * port calibration intialization from basalt?

### Timeline and todos orga for every year

- update website
- pre meeting
- docmatching ranking
- import in tumonline; limit time abmeldung; (maybe) enable waiting list
- send email about course start, abmeldung, preparation
- update actual dates in tumonline

- ...

- announce date of final presentation (before exam period) and report due date (after (most) exams)
- reminder final report
- before end of semester, remind people of deleted accounts (including gitlab)


### evalutation ws2018 comments improvements

- More object oriented software framework
- go deeper into algorithms and skip "plain programming" --> not sure we can go deeper given the time --> they can go deeper in projects
- some exercises can be solved without understanding --> mention at beginning goal of course (responsibility of students)
- give examples of past projects a course start so people can think about it longer
- better compters... 



### SFM ideas

 * ceres speed: test multi vs single threaded!

  // TODO: click on image to select track; highlight selected track in images
  // and 3D; cycle through other images with that track when continuing to
  // click;

  // TODO: allow removing just single observations, not only whole landmark
  // TODO: allow to later re-add wrongly removed landmarks
  // TODO: allow to later re-add wrongly removed observations
  // TODO: add or combine landmarks from outlier matches as well

  // TODO: load and display ground truth


 * add_new_landmarks_between_cams:
  // TODO: maybe check epipolar constraint before adding new landmarks to avoid some outliers from the start
  // TODO: Also check this for additional observations
  // TODO: Maybe triangulate from all observations, not just the 2,
  //       or better: Run robust point-only BA for new LM and remove
  //       outlier observations (not whole LM)
  // TODO: When removing outliers because of high repr error: remove only observations, and only remove landmark if observation count drops below 2 (or below minimal track count, but we have to consider not-yet-added cameras).


// TODO: update pnp thresh to not have conversion inside (maybe?); otherwise update
// 5pt thresh accordingly for next year.

 * ransac threshold:
  // TODO: consider computing this threshold per-point using the derivative
  //       of the projection function; per pixel also allows to properly deal
  //       with multi-scale feature detection.
