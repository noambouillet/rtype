# Interface

`engine::ui` draws menus and HUD elements in **screen space**: they stay in
place when the world view moves or the window is resized. Elements are ECS
entities built from small components, updated and drawn by three systems.

---

## 1. Render Spaces

`IGraphicsBackend::setRenderSpace` selects the coordinates of the next draw calls:

| Space | Used by | On window resize |
| --- | --- | --- |
| `RenderSpace::World` | `RenderSystem` (the game) | keeps its size and is stretched |
| `RenderSpace::Screen` | `UiRenderSystem`, `MetricsOverlay` | follows the window, one unit per pixel |

`beginFrame()` always starts in world space. `screenSize()` gives the size of
the screen space and `measureText()` the bounds of a text, used to align it.

---

## 2. Components

An element is the combination of its components: a button is a `UiTransform`,
a `UiRect` and a `UiText`, plus a `UiButton` to make it clickable.

| Component | Role |
| --- | --- |
| `UiTransform` | Anchor, offset, size and layer. Required: no system sees an entity without it |
| `UiRect` | Fills the element with a color |
| `UiImage` | Draws a texture region, scaled to fit the element without stretching and centered |
| `UiText` | Draws a text aligned left, center or right, centered vertically |
| `UiButton` | Makes the element clickable; `hovered` and `pressed` are written by `UiInputSystem` |
| `UiFocusable` | Lets the keyboard or joystick reach the button, in `order` |
| `UiButtonStyle` | Image region for the normal, hovered and pressed states |
| `UiColorStyle` | Rectangle color for the normal, hovered and pressed states |

The anchor is both a point of the screen and the point of the element placed on
it: a `BottomRight` element with no offset touches the bottom right corner, a
`Center` element is centered whatever its size.

---

## 3. Systems

| System | Signature | Role |
| --- | --- | --- |
| `UiInputSystem` | `UiTransform`, `UiButton` | Hover and press state, focus navigation, publishes `UiClicked` |
| `UiButtonStyleSystem` | `UiButton` | Copies the style of the current state into `UiImage` or `UiRect` |
| `UiRenderSystem` | `UiTransform` | Draws every element in screen space, by layer: rectangle, image, then text |

`registerInterface`, `updateInterface` and `renderInterface` in
`engine/ui/Interface.hpp` register and run them on a coordinator.

### Pointer and focus

The mouse comes from `IInputBackend::pointer()`, in screen coordinates. Only
the button with the highest layer under the pointer is hovered. A click is a
press then a release on the same button.

`UiInputSystem::setNavigation(previous, next, confirm)` names the input actions
used without a mouse. A navigation action moves the focus between the
`UiFocusable` buttons, the focused button is shown as hovered, and the confirm
action publishes the same `UiClicked` as a mouse click. Moving the mouse gives
control back to the pointer.

---

## 4. Example

```cpp
engine::ui::registerInterface(coordinator);
coordinator.getSystem<engine::ui::UiInputSystem>().setNavigation("ui_up", "ui_down", "ui_confirm");

const engine::core::Entity play = coordinator.createEntity();
coordinator.addComponent(play, engine::ui::UiTransform{.anchor = engine::ui::Anchor::Center, .offsetX = 0.0F, .offsetY = 0.0F, .width = 320.0F, .height = 56.0F, .layer = 0});
coordinator.addComponent(play, engine::ui::UiRect{.color = {.r = 30, .g = 34, .b = 70, .a = 230}});
coordinator.addComponent(play, engine::ui::UiColorStyle{.normal = {.r = 30, .g = 34, .b = 70, .a = 230}, .hovered = {.r = 70, .g = 80, .b = 160, .a = 255}, .pressed = {.r = 18, .g = 20, .b = 45, .a = 255}});
coordinator.addComponent(play, engine::ui::UiText{.font = font, .text = "Play", .size = 26, .color = {.r = 240, .g = 240, .b = 240, .a = 255}, .align = engine::ui::TextAlign::Center});
coordinator.addComponent(play, engine::ui::UiButton{.action = "play", .hovered = false, .pressed = false});
coordinator.addComponent(play, engine::ui::UiFocusable{.order = 0});

auto onClick = bus.subscribe<engine::ui::UiClicked>([](const engine::ui::UiClicked& click) {
	// click.action == "play"
});

// Each frame
engine::ui::updateInterface(coordinator, input, graphics, bus);
engine::ui::renderInterface(coordinator, graphics);
```

Clicks are published after every button is updated, so a handler may create or
destroy interface entities.
