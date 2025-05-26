#!/bin/bash
set -e

DEV_BRANCH="dev"
MAIN_BRANCH="main"

echo "⏩ Przełączam na główną gałąź: $MAIN_BRANCH"
git checkout $MAIN_BRANCH

echo "⬇️ Pobieram najnowsze zmiany z remote"
git pull origin $MAIN_BRANCH

echo "🔄 Pobieram zmiany z gałęzi $DEV_BRANCH"
git fetch origin $DEV_BRANCH

echo "🔀 Merge $DEV_BRANCH do $MAIN_BRANCH"
git merge origin/$DEV_BRANCH

echo "⬆️ Pushuję zmergowaną gałąź $MAIN_BRANCH do remote"
git push origin $MAIN_BRANCH

echo "✅ Merge zakończony!"

