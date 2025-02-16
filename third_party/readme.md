# Third party repos

## gitlab-gantt-maker
How to use:
1. apply the patch for using only one project (among other visual tweaks):
  ```
  cd gitlab-gantt-maker && git apply ../gantt_patch.diff
  ```
2. Install the requirements
  ```
  pip3 install -r requirements.txt
  ```
3. Configure the project in config.ini: (group doesn't matter)
  ```
  Instance = https://git.ucsc.edu
  PersonalAccessToken = <your api token here>
  ```
4. run
  ```
  python3 gitlab-gantt-maker.py
  ```
The result will be in gantt.html
