#/bin/bash 


if grep -q openkorp-message-set ".gitmodules"; then
  echo "Found openkorp message set submodule"
else 
  echo "Missing openkorp message set, adding submodule"
  git submodule --quiet add https://github.com/openKorp/openkorp-message-set.git
fi

if grep -q opendlv.standard-message-set ".gitmodules"; then
  echo "Found opendlv message set submodule"
else 
  echo "Missing opendlv message set, adding submodule"
  git submodule --quiet add https://github.com/chalmers-revere/opendlv.standard-message-set.git
fi

git submodule -q update --init --recursive
git submodule -q init
git submodule -q sync
git submodule -q update --remote

echo "Updating message sets"

cp openkorp-message-set/openkorp-message-set.odvd src/openkorp-message-set.odvd
cp opendlv.standard-message-set/opendlv.odvd src/opendlv-standard-message-set.odvd
