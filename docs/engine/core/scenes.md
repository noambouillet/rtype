# Scenes

A scene is one screen of a game: title menu, game, pause menu. The
`SceneManager` keeps them on a stack, so a menu can be opened over the game and
closed to get back to it.

---

## 1. Stack

| Call | Effect |
| --- | --- |
| `push(scene)` | Puts a scene on top; the one below gets `onPause()` |
| `pop()` | Removes the top scene; the one below gets `onResume()` |
| `replace(scene)` | Swaps the top scene for another one |
| `clear()` | Removes every scene; the game stops once the stack is empty |

These calls are applied at the end of `SceneManager::update`, in the order they
were made. A scene can therefore ask to be replaced from one of its own
methods, such as a button handler, without being destroyed while it runs.

---

## 2. Frame

```text
SceneManager::update(deltaTime)
    top scene      handleInput()
    every scene    update(deltaTime)      bottom to top
    pending push / pop / replace / clear
SceneManager::render()
    every scene    render()               bottom to top
```

Only the top scene reads the input, but every scene keeps being updated and
drawn. In a networked game, the game scene under the pause menu keeps talking
to the server and stays visible behind it.

| Hook | Called |
| --- | --- |
| `onEnter()` | Once the scene is on the stack, before its first update |
| `onExit()` | Before the scene is removed and destroyed |
| `onPause()` | When another scene is pushed over it |
| `onResume()` | When the scene above it is popped |

The manager's destructor runs `onExit()` for every scene left. When scenes hold
references to objects declared after the manager, call `clear()` then
`update(0.0F)` before those objects are destroyed.

---

## 3. R-Type Scenes

| Scene | Content |
| --- | --- |
| `MainMenuScene` | Scrolling background, Play and Quit buttons, and the reason the last game ended |
| `GameScene` | Socket, `GameClient` and `ClientWorld`; the pause action pushes `PauseScene` |
| `PauseScene` | Dark layer over the game, Resume, Main menu and Quit buttons; the back action resumes |

Each scene owns its coordinator and an event bus for its `UiClicked`, so the
buttons of one scene never trigger the handlers of another.
