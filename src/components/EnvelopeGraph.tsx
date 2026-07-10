import { useRef } from 'react'
import type { Envelope } from '../state/types'
import { useStore } from '../state/store'

interface Props {
  ep: Envelope
  w: number
  h: number
  onChange?: (ep: Envelope) => void
}

const clamp01 = (v: number) => Math.max(0, Math.min(1, v))

/**
 * ADSR display — ported from the handoff's env() SVG builder, drawn on the
 * LCD/LED display field. When `onChange` is supplied, the attack / decay /
 * sustain / release nodes are draggable and update the envelope live.
 */
export function EnvelopeGraph({ ep, w, h, onChange }: Props) {
  const svgRef = useRef<SVGSVGElement>(null)
  const px = 5, py = 6, iw = w - px * 2, ih = h - py * 2, yb = h - py

  const xa = px + iw * (0.03 + ep.a * 0.24)
  const xd = Math.min(xa + iw * (0.05 + ep.d1 * 0.3), px + iw * 0.66)
  const ys = py + (1 - ep.s) * ih
  const xs = px + iw * 0.78
  const xr = Math.min(xs + iw * (0.05 + ep.r * 0.17), px + iw)
  const P: [number, number][] = [[px, yb], [xa, py], [xd, ys], [xs, ys], [xr, yb]]

  // node index (into P) -> handler that maps a viewBox point to a new envelope
  const drag = (nodeIdx: number) => (e: React.PointerEvent) => {
    if (!onChange) return
    e.stopPropagation()
    const target = e.currentTarget as Element
    target.setPointerCapture(e.pointerId)
    useStore.getState().beginInteraction()
    const move = (ev: PointerEvent) => {
      const rect = svgRef.current!.getBoundingClientRect()
      const x = ((ev.clientX - rect.left) / rect.width) * w
      const y = ((ev.clientY - rect.top) / rect.height) * h
      const next: Envelope = { ...ep }
      if (nodeIdx === 1) {
        next.a = clamp01(((x - px) / iw - 0.03) / 0.24)
      } else if (nodeIdx === 2) {
        next.d1 = clamp01(((x - xa) / iw - 0.05) / 0.3)
        next.s = clamp01(1 - (y - py) / ih)
      } else if (nodeIdx === 3) {
        next.s = clamp01(1 - (y - py) / ih)
      } else if (nodeIdx === 4) {
        next.r = clamp01(((x - xs) / iw - 0.05) / 0.17)
      }
      onChange(next)
    }
    const up = (ev: PointerEvent) => {
      target.releasePointerCapture(ev.pointerId)
      useStore.getState().endInteraction()
      window.removeEventListener('pointermove', move)
      window.removeEventListener('pointerup', up)
    }
    window.addEventListener('pointermove', move)
    window.addEventListener('pointerup', up)
  }

  const pts = P.map((q) => q.join(',')).join(' ')
  const draggable = [false, true, true, true, true] // origin node fixed

  return (
    <svg
      ref={svgRef}
      width="100%"
      height={h}
      viewBox={`0 0 ${w} ${h}`}
      preserveAspectRatio="none"
      style={{ display: 'block', background: 'var(--disp)', borderRadius: 3, border: '1px solid var(--disp-b)', boxSizing: 'border-box', boxShadow: 'var(--disp-inset)' }}
    >
      <line x1={px} y1={yb} x2={w - px} y2={yb} style={{ stroke: 'var(--disp-ink)' }} strokeWidth={1} opacity={0.25} />
      <path d={`M${pts} Z`} style={{ fill: 'var(--disp-ink)' }} opacity={0.11} />
      <polyline points={pts} fill="none" style={{ stroke: 'var(--disp-ink)' }} strokeWidth={1.7} strokeLinejoin="round" />
      {P.map((q, i) => (
        <circle
          key={i}
          cx={q[0]}
          cy={q[1]}
          r={3}
          style={{ fill: 'var(--disp-dot)', stroke: 'var(--disp-ink)', cursor: onChange && draggable[i] ? 'grab' : 'default', touchAction: 'none' }}
          strokeWidth={1.4}
          onPointerDown={onChange && draggable[i] ? drag(i) : undefined}
        />
      ))}
    </svg>
  )
}

/** Pitch-EG mini graph — ported from the handoff's peg() builder (display-only). */
export function PitchEgGraph({ w, h }: { w: number; h: number }) {
  const mid = h / 2
  const P: [number, number][] = [[5, mid], [w * 0.28, h * 0.18], [w * 0.58, h * 0.78], [w - 5, mid]]
  const pts = P.map((q) => q.join(',')).join(' ')
  return (
    <svg width="100%" height={h} viewBox={`0 0 ${w} ${h}`} preserveAspectRatio="none" style={{ display: 'block', background: 'var(--disp)', borderRadius: 3, border: '1px solid var(--disp-b)', boxSizing: 'border-box', boxShadow: 'var(--disp-inset)' }}>
      <line x1={4} y1={mid} x2={w - 4} y2={mid} style={{ stroke: 'var(--disp-ink)' }} strokeWidth={1} strokeDasharray="3 3" opacity={0.3} />
      <polyline points={pts} fill="none" style={{ stroke: 'var(--disp-ink)' }} strokeWidth={1.7} strokeLinejoin="round" />
      {P.map((q, i) => (
        <circle key={i} cx={q[0]} cy={q[1]} r={2.6} style={{ fill: 'var(--disp-dot)', stroke: 'var(--disp-ink)' }} strokeWidth={1.3} />
      ))}
    </svg>
  )
}
