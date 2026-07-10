import { useRef } from 'react'
import { useStore } from '../state/store'

interface Props {
  value: number
  onChange?: (v: number) => void
  min?: number
  max?: number
  height?: number
}

/** Horizontal fill slider (Generator count/mutation). Click or drag to set. */
export function Slider({ value, onChange, min = 0, max = 100, height = 5 }: Props) {
  const ref = useRef<HTMLDivElement>(null)
  const pct = ((value - min) / (max - min)) * 100

  const setFromEvent = (clientX: number) => {
    if (!onChange || !ref.current) return
    const rect = ref.current.getBoundingClientRect()
    const r = Math.max(0, Math.min(1, (clientX - rect.left) / rect.width))
    onChange(Math.round(min + r * (max - min)))
  }

  const onDown = (e: React.PointerEvent) => {
    if (!onChange) return
    const el = e.currentTarget as HTMLElement
    el.setPointerCapture(e.pointerId)
    useStore.getState().beginInteraction()
    setFromEvent(e.clientX)
    const move = (ev: PointerEvent) => setFromEvent(ev.clientX)
    const up = (ev: PointerEvent) => {
      el.releasePointerCapture(ev.pointerId)
      useStore.getState().endInteraction()
      window.removeEventListener('pointermove', move)
      window.removeEventListener('pointerup', up)
    }
    window.addEventListener('pointermove', move)
    window.addEventListener('pointerup', up)
  }

  return (
    <div
      ref={ref}
      onPointerDown={onDown}
      style={{ height, background: 'var(--inset)', borderRadius: 3, overflow: 'hidden', cursor: onChange ? 'pointer' : 'default', touchAction: 'none' }}
    >
      <div style={{ height: '100%', width: `${pct}%`, background: 'var(--acc)' }} />
    </div>
  )
}
