import { useRef } from 'react'
import { deg } from '../theme/tokens'
import { useStore } from '../state/store'

interface Props {
  value: number // 0..99
  onChange?: (v: number) => void
  size?: number
  pointerLen?: number
  borderColor?: string // defaults to var(--knob-b)
  pointerColor?: string // defaults to var(--ptr)
  min?: number
  max?: number
}

/**
 * Rotary knob — the dial only (value readouts live alongside it, matching the
 * handoff markup). Vertical drag scrubs the value; right-click is reserved for
 * the "assign CC / add to Sizzler" context menu (known open item — not drawn).
 */
export function Knob({ value, onChange, size = 32, pointerLen, borderColor = 'var(--knob-b)', pointerColor = 'var(--ptr)', min = 0, max = 99 }: Props) {
  const len = pointerLen ?? Math.round(size * 0.375)
  const startRef = useRef({ y: 0, v: 0 })

  const onDown = (e: React.PointerEvent) => {
    if (!onChange) return
    e.preventDefault()
    const el = e.currentTarget as HTMLElement
    el.setPointerCapture(e.pointerId)
    startRef.current = { y: e.clientY, v: value }
    useStore.getState().beginInteraction()
    const range = max - min
    const move = (ev: PointerEvent) => {
      const dy = startRef.current.y - ev.clientY // up = increase
      const next = startRef.current.v + (dy / 150) * range
      onChange(Math.max(min, Math.min(max, Math.round(next))))
    }
    const up = (ev: PointerEvent) => {
      el.releasePointerCapture(ev.pointerId)
      useStore.getState().endInteraction()
      window.removeEventListener('pointermove', move)
      window.removeEventListener('pointerup', up)
    }
    window.addEventListener('pointermove', move)
    window.addEventListener('pointerup', up)
  }

  const onContext = (e: React.MouseEvent) => {
    // TODO: draw the "assign CC / add to Sizzler" context menu here.
    e.preventDefault()
  }

  const rot = deg(((value - min) / (max - min)) * 99)

  return (
    <div
      onPointerDown={onDown}
      onContextMenu={onContext}
      style={{
        position: 'relative', width: size, height: size, borderRadius: '50%',
        background: 'var(--knob)', border: `1px solid ${borderColor}`,
        boxShadow: 'inset 0 2px 4px rgba(0,0,0,.5)',
        cursor: onChange ? 'ns-resize' : 'default', touchAction: 'none', flex: 'none',
      }}
    >
      <div style={{ position: 'absolute', left: '50%', top: '50%', width: 2, height: len, margin: `${-len}px 0 0 -1px`, transformOrigin: '50% 100%', transform: `rotate(${rot}deg)`, background: pointerColor, borderRadius: 1 }} />
    </div>
  )
}
