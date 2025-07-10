### Build
Checkout the Moppy2 repo.  

In the root of the repo build the docker image  
`docker build -f docker/Dockerfile -t moppy2:latest .`

### Run  
`docker run -ti --rm -d --privileged -e USER_ID=0 -e GROUP_ID=0 -p 5800:5800 -v "my/local/midi:/moppy2/songs" moppy2:latest`

### Access
Open you browser and go to
`http://127.0.0.1:5800`
